import argparse
import sys
import os
from _pxx import Parser, Module

def main():
    ###########################################################################
    # Command line arguments
    ###########################################################################
    parser = argparse.ArgumentParser(prog="pxx",
                                    description=
                                    """
                                    Automatic generation of Python interface
                                    module from C++ source files.
                                    """,
                                    epilog=
                                    """
                                    Remaining arguments are passed on to clang.
                                    """)
    parser.add_argument("input_file",
                        nargs=1,
                        metavar=("<input_file>",),
                        help="The C++ file containing the classes for which to "
                        " generate the interface.")
    parser.add_argument("--dump_ast",
                        action='store_true',
                        help="If provided, pxx only dumps the AST of the parsed"
                        " translation unit.")
    parser.add_argument("--classes",
                        nargs="*",
                        metavar="Class1, ...",
                        help="List of classes to include in the module "
                        " interface.")
    parser.add_argument("--module_name",
                        nargs=1,
                        metavar="name",
                        help="Python module name use for the top-level "
                        "namespace.")
    parser.add_argument("--output_file",
                        nargs=1,
                        metavar=("<output_file>",),
                        help="File to write the module code to.")
    parser.add_argument("--export_all",
                        help="Whether or not to export all found definitions.",
                        action="store_true")

    args, other_args = parser.parse_known_args()
    input_file = args.input_file[0]
    classes = args.classes
    output_file = args.output_file
    module_name = args.module_name
    export_all = args.export_all

    ###########################################################################
    # Check arguments
    ###########################################################################

    if not os.path.exists(input_file):
        print("The provided input file '{}' does not exist.".format(input_file))
        return 1

    if not output_file is None:
        output_file = output_file[0]
        dirname = os.path.dirname(output_file)
        if not os.path.exists(dirname):
            print("The provided output directory '{}' does not exist."
                  .format(dirname))
            return 1

    if module_name is None:
        if output_file is None:
            name = os.path.basename(input_file)
        else:
            name = os.path.basename(output_file)
        module_name, _ = os.path.splitext(name)
    else:
        module_name = module_name[0]

    ############################################################################
    # Command line arguments
    ############################################################################

    include_path = os.path.join(os.path.dirname(__file__), "include", "clang")

    print("other args: ", other_args)
    parser = Parser(input_file, [f"-I{include_path}"] + other_args)
    if export_all:
        parser.set_export_default()
    tu = parser.parse()

    # Only dumping AST.
    if args.dump_ast:
        print(f"\n### CXX Translation Unit: {input_file} ###\n")
        tu.dump_ast()
        tu.print()
        return 0

    module = Module(module_name, [os.path.basename(input_file)])
    s = module.render(tu)

    # Write output
    if output_file is None:
        print(s)
    else:
        with open(output_file, "w") as f:
            f.write(s)

    return 0

if __name__ == '__main__':
    main()
