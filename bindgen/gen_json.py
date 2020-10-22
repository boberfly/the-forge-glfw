#-------------------------------------------------------------------------------
#   Read an AST file generated with:
#
#   clang -Xclang -ast-dump=json header.h >header.ast.sjon
#
#   ...and generate a simplified JSON file with the public API declaration.
#-------------------------------------------------------------------------------
import json
import sys

def is_api_decl(decl):
    if 'loc' in decl:
        if 'includedFrom' in decl['loc']:
            return False
        if 'spellingLoc' in decl['loc']:
            if 'includedFrom' in decl['loc']['spellingLoc']:
                return False
    if 'name' in decl:
        if decl['name'].startswith('__'):
            return False
        elif decl['name'] == 'COMPILE_ASSERT':
            return False
        elif decl['name'] == 'static_assert':
            return False
        elif decl['name'] == 'MAKE_ENUM_FLAG':
            return False
    if 'storageClass' in decl:
        if decl['storageClass'] == 'extern':
            return False

    return True

def is_api_struct(decl):
    if not 'inner' in decl:
        return False
    return True

def filter_types(str):
    return str.split("::")[0]
    
def parse_embedded_struct(decl, item, recurse=2):
    if recurse == 0:
        return
    item['fields'] = []
    if 'name' in decl:
        item['name'] = decl['name']
    for item_decl in decl['inner']:
        item2 = {}
        if item_decl['kind'] == 'RecordDecl':
            parse_embedded_struct(item_decl, item2, recurse-1)
        elif item_decl['kind'] == 'FieldDecl':
            if 'name' in item_decl:
                item2['name'] = item_decl['name']
            if 'type' in item_decl:
                item2['type'] = filter_types(item_decl['type']['qualType'])
        else:
            continue
        item['fields'].append(item2)

    if 'tagUsed' in decl:
        item['kind'] = decl['tagUsed']

def parse_struct(decl):
    if not is_api_struct(decl):
        return None
    outp = {}
    outp['kind'] = 'struct'
    outp['name'] = decl['name']
    outp['fields'] = []
    if 'tagUsed' in decl:
        if decl['tagUsed'] == "union":
            outp['union'] = True
    for item_decl in decl['inner']:
        item = {}
        if item_decl['kind'] == 'RecordDecl':
            # Some structs have structs in them
            parse_embedded_struct(item_decl, item)
        elif item_decl['kind'] == 'TypedefDecl':
            item['name'] = item_decl['name']
            print(item['name'])
        elif item_decl['kind'] == 'FieldDecl':
            if 'name' in item_decl:
                item['name'] = item_decl['name']
            if 'type' in item_decl:
                item['type'] = filter_types(item_decl['type']['qualType'])
            # Bitfield values
            if 'inner' in item_decl:
                if 'valueCategory' in item_decl['inner'][0]:
                    if item_decl['inner'][0]['valueCategory'] == 'rvalue':
                        item['bitfieldvalue'] = item_decl['inner'][0]['inner'][0]['value']
            # Default values seem to not be picked up by the AST...
            if 'name' in decl:
                if decl['name'] == 'DescriptorData':
                    if 'name' in item:
                        if item['name'] == 'mIndex':
                            item['defaultvalue'] = '(uint32_t)-1';
                        elif item['name'] == 'mExtractBuffer':
                            item['defaultvalue'] = 'false';
        else:
            continue
        outp['fields'].append(item)
    return outp

def parse_enum(decl, hdr):
    outp = {}
    if 'name' in decl:
        outp['kind'] = 'enum'
        outp['name'] = decl['name']
        needs_value = False
    else:
        outp['kind'] = 'consts'
        needs_value = True
    outp['items'] = []
    if 'inner' not in decl:
        return None
    for item_decl in decl['inner']:
        if item_decl['kind'] == 'EnumConstantDecl':
            item = {}
            item['name'] = item_decl['name']
            if 'inner' in item_decl:
                const_expr = item_decl['inner'][0]
                # We really need to just scrape the header itself
                isMetal = False
                for line in hdr:
                    if "#ifdef METAL" in line:
                        isMetal = True
                    elif isMetal and "#else" in line:
                        isMetal = False
                    elif isMetal and "#endif" in line:
                        isMetal = False
                    if item['name'] in line:
                        # Skip over due to #ifdef METAL
                        if isMetal and "SHADER_STAGE" in line:
                            continue
                        if " = " in line:
                            enum = line.lstrip()
                            esplit = enum.split(" = ")
                            item['value'] = esplit[-1].split(",")[0].split("//")[0].rstrip()
                            break

            outp['items'].append(item)
    return outp

def parse_func(decl):
    outp = {}
    outp['kind'] = 'func'
    outp['name'] = decl['name']
    outp['type'] = filter_types(decl['type']['qualType'])
    outp['params'] = []
    if 'inner' in decl:
        for param in decl['inner']:
            if not is_api_decl(param):
                continue
            if param['kind'] != 'ParmVarDecl':
                continue
                #sys.exit(f"ERROR: func param kind must be 'ParmVarDecl' ({decl['name']})")
            outp_param = {}
            if 'name' in param:
                outp_param['name'] = param['name']
                outp_param['type'] = filter_types(param['type']['qualType'])
                outp['params'].append(outp_param)
    return outp

def parse_decl(decl, hdr):
    kind = decl['kind']
    if kind == 'RecordDecl':
        return parse_struct(decl)
    elif kind == 'EnumDecl':
        return parse_enum(decl, hdr)
    elif kind == 'FunctionDecl':
        return parse_func(decl)
    else:
        return None

def parse_ast(ast, hdr):
    outp = {}
    outp['decls'] = []
    for decl in ast['inner']:
        if is_api_decl(decl):
            outp_decl = parse_decl(decl, hdr)
            if outp_decl is not None:
                outp['decls'].append(outp_decl)
  
    return outp

def gen_json(path):
    input_path = path + ".ast.json"
    header_path = path + ".h"
    output_path = path + ".json"
    try:

        print(f">>> {input_path} + {header_path} => {output_path}")
        with open(header_path, "rt") as header:
            hdr = []
            for line in header:
                hdr.append(line)
            with open(input_path, 'r') as f_inp:
                inp = json.load(f_inp)
                outp = parse_ast(inp, hdr)
            with open(output_path, 'w') as f_outp:
                json.dump(outp, f_outp, indent='  ')
    except EnvironmentError as err:
        print(f"{err}")

def main():
    gen_json('IRay')
    gen_json('IRenderer')


if __name__ == '__main__':
    main()
