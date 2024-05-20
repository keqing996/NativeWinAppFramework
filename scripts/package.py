import os
import zipfile
import sys


def add_folder_to_zip(zipf: zipfile.ZipFile, folder_path: str, zip_folder_name: str):
    for root, _, files in os.walk(folder_path):
        for file in files:
            zipf.write(os.path.join(root, file), os.path.realpath(os.path.join(root, file)), os.path.join(folder_path, '..'))


def main(lib_name: str, debug_lib_path: str, release_lib_path: str):

    zip_path: str = '../package/'
    if not os.path.exists(zip_path):
        os.makedirs(zip_path)

    # make debug zip
    with zipfile.ZipFile(zip_path + 'debug', 'w', zipfile.ZIP_DEFLATED) as zipf:
        add_folder_to_zip(zipf, '../include/', 'include')
        zipf.write(debug_lib_path, 'lib/' + lib_name)

    # make release zip
    with zipfile.ZipFile(zip_path + 'debug', 'w', zipfile.ZIP_DEFLATED) as zipf:
        add_folder_to_zip(zipf, '../include/', 'include')
        zipf.write(release_lib_path, 'lib/' + lib_name)


if __name__ == '__main__':
    lib_name: str = sys.argv[1]
    debug_lib_path: str = sys.argv[2]
    release_lib_path: str = sys.argv[3]
    main(lib_name, debug_lib_path, release_lib_path)
