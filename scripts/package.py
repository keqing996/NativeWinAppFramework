import os
import zipfile


def add_folder_to_zip(zipf: zipfile.ZipFile, folder_path: str, zip_folder_name: str):
    for root, _, files in os.walk(folder_path):
        for file in files:
            zipf.write(os.path.join(root, file), os.path.relpath(os.path.join(root, file), os.path.join(folder_path, '..')))


def main(lib_name: str, debug_lib_path: str, release_lib_path: str):

    zip_path: str = '../package/'
    if not os.path.exists(zip_path):
        os.makedirs(zip_path)

    with zipfile.ZipFile(zip_path + 'native_win_app.zip', 'w', zipfile.ZIP_DEFLATED) as zipf:
        add_folder_to_zip(zipf, '../include/', 'include')
        zipf.write(debug_lib_path, 'debug/lib/' + lib_name)
        zipf.write(release_lib_path, 'release/lib/' + lib_name)


if __name__ == '__main__':
    lib_name: str = "native_win_app.lib"
    debug_lib_path: str = "../artifacts/MSVC/Debug/cpp_native_win_app.lib"
    release_lib_path: str = "../artifacts/MSVC/Release/cpp_native_win_app.lib"
    main(lib_name, debug_lib_path, release_lib_path)
