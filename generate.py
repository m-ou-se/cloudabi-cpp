#!/usr/bin/env python3
# Copyright (c) 2016 Nuxi (https://nuxi.nl/) and contributors.
#
# This file is distributed under a 2-clause BSD license.
# See the LICENSE and CONTRIBUTORS files for details.

from contextlib import redirect_stdout
import io
import os
import subprocess

from cloudabi.generator.abi import *
from cloudabi.generator.c import *
from cloudabi.generator.parser import *


class CppNaming:

    def mangle(self, name):
        if name[0].isdigit():
            return '_' + name
        if name in {'private', 'int', 'delete'}:
            return name + '_'
        if name == 'errno':
            return 'error'
        return name

    def typename(self, type):
        if isinstance(type, VoidType):
            return 'void'
        elif isinstance(type, IntType):
            if type.name == 'char':
                return 'char'
            return 'std::{}_t'.format(type.name)
        elif isinstance(type, UserDefinedType):
            return self.mangle(type.name)
        elif isinstance(type, AtomicType):
            return 'std::atomic<{}>'.format(
                self.typename(type.target_type))
        elif isinstance(type, PointerType) or isinstance(type, ArrayType):
            return self.vardecl(type, '')
        else:
            raise Exception('Unable to generate C++ declaration '
                            'for type: {}'.format(type))

    def valname(self, type, value):
        return self.mangle(value.name)

    def syscallname(self, syscall):
        return self.mangle(syscall.name)

    def vardecl(self, type, name, array_need_parens=False):
        if isinstance(type, PointerType):
            decl = self.vardecl(type.target_type, '*{}'.format(name),
                                array_need_parens=True)
            if type.const:
                decl = 'const ' + decl
            return decl
        elif isinstance(type, ArrayType):
            if array_need_parens:
                name = '({})'.format(name)
            return self.vardecl(
                type.element_type, '{}[{}]'.format(
                    name, type.count))
        else:
            return '{} {}'.format(self.typename(type), name)


class CppGenerator(CGenerator):

    def __init__(self, cnaming, **kwargs):
        super().__init__(**kwargs, naming=None)
        self.naming = CppNaming()
        self.cnaming = cnaming

    def generate_head(self, abi):
        super().generate_head(abi)
        print('namespace cloudabi {\n')

    def generate_foot(self, abi):
        print('}\n')
        super().generate_foot(abi)


class CppTypeDefsGenerator(CppGenerator):

    def __init__(self, skip = set(), **kwargs):
        super().__init__(**kwargs)
        self.skip = skip


    def generate_type(self, abi, type):

        name = self.naming.typename(type)
        cname = self.cnaming.typename(type)

        if type.name in self.skip:
            return

        if isinstance(type, AliasType):
            print('using {} = {};\n'.format(name, cname))
        elif isinstance(type, IntLikeType):
            print('enum class {} : {} {{'.format(name, cname), end='')
            values = type.values
            if len(type.values) > 0 or isinstance(type, FlagsType):
                print()
                width = max(len(self.naming.valname(type, v)) for v in type.values)
                if isinstance(type, FlagsType):
                    print('  {name:{width}} = 0,'.format(name='none', width=width))
                for v in type.values:
                    print('  {name:{width}} = {val},'.format(
                            name=self.naming.valname(type, v),
                            width=width,
                            val=self.cnaming.valname(type, v)))
            print('};\n')
            if isinstance(type, FlagsType):
                for op in ('|', '&'):
                    print('inline {type} operator {op} ({type} a, {type} b) {{\n'
                          '  return {type}({ctype}(a) {op} {ctype}(b));\n'
                          '}}\n'.format(op=op, type=name, ctype=cname))
                print('inline {type} operator ~ ({type} v) {{\n'
                      '  return {type}(~{ctype}(v));\n'
                      '}}\n'.format(op=op, type=name, ctype=cname))
        elif isinstance(type, StructType):
            print('struct {};\n'.format(name))


class CppStructDefsGenerator(CppGenerator):

    def __init__(self, skip = set(), **kwargs):
        super().__init__(**kwargs)
        self.skip = skip

    def generate_struct_members(self, abi, type, indent='', reserved_names = set()):
        names = set()
        for m in type.members:
            if m.name is not None:
                names.add(m.name)
            if isinstance(m, SimpleStructMember):
                print('{}{}{};'.format(
                    indent, 'cloudabi::' if m.type.name in reserved_names else '',
                    self.naming.vardecl(m.type, m.name)))
            elif isinstance(m, VariantStructMember):
                print('{}union {{'.format(indent))
                for x in m.members:
                    if x.name is None:
                        self.generate_struct_members(
                            abi, x.type, indent + '  ', names)
                    else:
                        print('{}  struct {{'.format(indent))
                        self.generate_struct_members(
                            abi, x.type, indent + '    ', names)
                        print('{}  }} {};'.format(indent, x.name))
                print('{}}};'.format(indent))
            elif isinstance(m, RangeStructMember):
                print('{}mstd::range<{}{}> {};'.format(
                    indent, self.naming.typename(m.target_type),
                    ' const' if m.const else '', m.name))
            else:
                raise Exception('Unknown struct member: {}'.format(m))

    def generate_struct_asserts(self, abi, type):
        name = self.naming.typename(type)
        cname = self.cnaming.typename(type)
        print('static_assert(sizeof({}) == sizeof({}), "");'.format(name, cname))
        print('static_assert(alignof({}) == alignof({}), "");'.format(name, cname))
        self.generate_struct_member_asserts(abi, name, cname, type.members, '')

    def generate_struct_member_asserts(self, abi, name, cname, members, path):
        for m in members:
            if isinstance(m, SimpleStructMember):
                memname = path + m.name
                print('static_assert(offsetof({}, {}) == offsetof({}, {}), "");'.format(
                    name, memname, cname, memname))
            elif isinstance(m, VariantStructMember):
                for x in m.members:
                    if x.name is None:
                        self.generate_struct_member_asserts(
                            abi, name, cname, x.type.members, '')
                    else:
                        self.generate_struct_member_asserts(
                            abi, name, cname, x.type.members, x.name + '.')
            elif isinstance(m, RangeStructMember):
                memname = path + m.name
                cmemname1 = path + m.base_name
                cmemname2 = path + m.length_name
                print('static_assert(offsetof({}, {}) == offsetof({}, {}), "");'.format(
                    name, memname, cname, cmemname1))

    def generate_types(self, abi, types):
        super().generate_types(abi, types)

    def generate_type(self, abi, type):
        if type.name in self.skip:
            return
        if isinstance(type, StructType):
            print('struct {} {{'.format(self.naming.typename(type)))
            self.generate_struct_members(abi, type, '  ')
            print('};')
            self.generate_struct_asserts(abi, type)
            print()


abi = AbiParser().parse_abi_file(
    os.path.join(os.path.dirname(__file__), 'cloudabi/cloudabi.txt'))

cpp_skip={
    'auxtype',
    'auxv',
    'ciovec',
    'dirent',
    'fd',
    'lookup',
    'iovec',
    'tcb',
    'threadattr',
}

with open('cloudabi_types.hpp', 'w') as f:
    with redirect_stdout(f):
        CppTypeDefsGenerator(
            cnaming=CNaming('cloudabi_'),
            header_guard='CLOUDABI_TYPES_HPP',
            preamble='#include <cloudabi_types.h>\n',
            skip=cpp_skip
        ).generate_abi(abi)

with open('cloudabi_structs.hpp', 'w') as f:
    with redirect_stdout(f):
        CppStructDefsGenerator(
            cnaming=CNaming('cloudabi_'),
            header_guard='CLOUDABI_STRUCTS_HPP',
            preamble='#include <atomic>\n'
                     '#include <cstddef>\n'
                     '#include <cstdint>\n\n'
                     '#include <mstd/range.hpp>\n\n'
                     '#include <cloudabi_types.h>\n\n'
                     '#include "cloudabi_types.hpp"\n'
                     '#include "cloudabi_fd.hpp"\n'
                     '#include "cloudabi_iovec.hpp"\n',
            skip=cpp_skip
        ).generate_abi(abi)
