#!/usr/bin/python3

import os
import subprocess

def gen():
    proto_path = "./"
    proto_path = os.path.abspath(proto_path)
    all_files = []

    protoc = "/data/muslEnv/vcpkg/packages/protobuf_x64-linux/tools/protobuf/protoc"
    for file_name in os.listdir(proto_path):
        file_bare_name, extension = os.path.splitext(file_name)
        if extension == ".proto":
            result = subprocess.run(
                [
                    protoc, 
                    "-I", proto_path, 
                    "--cpp_out=" + proto_path + "/gen",
                    proto_path + "/" + file_name
                ], 
                capture_output=True,
                text=True
            )
            if result.returncode != 0:
                print(result.stderr)
            else:
                print("parse\t[" + file_name + "]\tdone.")


if __name__ == "__main__":
    gen()