import os
import sys
import shutil

from shutil import copy2
from shutil import copyfile
from shutil import copytree

def collceion_program_files(type, force_update, publish, in_target_path):
    base_path = ""
    #target_folder_suffix = ""
    if type == "debug":
        base_path = "./../"
        #target_folder_suffix = "_debug"
    elif type == "release":
        base_path = "./../"
        #target_folder_suffix = "_server_windows"
    elif type == "rel-debug":
        base_path = "./../"
        #target_folder_suffix = "_server_windows_dbginfo"
    else:
        print("don't known the mode : {}, must debug/release".format(sys.argv[1]))
        return

    print("the path is : {}".format(base_path))

    ignore_files = [
        "plugin_amf_encoder.dll",
        "plugin_clipboard.dll",
        "plugin_dda_capture.dll",
        "plugin_ffmpeg_encoder.dll",
        "plugin_file_transfer.dll",
        "plugin_frame_debugger.dll",
        "plugin_frame_resizer.dll",
        "plugin_media_recorder.dll",
        "plugin_mock_video_stream.dll",
        "plugin_net_relay.dll",
        "plugin_net_rtc.dll",
        "plugin_net_udp.dll",
        "plugin_net_ws.dll",
        "plugin_nvenc_encoder.dll",
        "plugin_obj_detector.dll",
        "plugin_opus_encoder.dll",
        "plugin_was_audio_capture.dll",
        "plugin_vr_manager.dll",
        "tc_rtc_client.dll",
        "protoc.exe"
    ]

    files_with_ref_path = []
    files = os.listdir(base_path)
    for file in files:
        found = False
        for ignore_file in ignore_files:
            if file == ignore_file:
                found = True
                break
        if found:
            continue

        file_path = base_path + "/" + file
        if ".dll" in file:
            files_with_ref_path.append(file_path)
        if ".DLL" in file:
            files_with_ref_path.append(file_path)
        if ".exe" in file  and "vc_redist.x64.exe" not in file:
            files_with_ref_path.append(file_path)
        if ".key" in file:
            files_with_ref_path.append(file_path)
        if ".toml" in file:
            files_with_ref_path.append(file_path)
        if ".ico" in file:
            files_with_ref_path.append(file_path)
        if not publish:
            if "data.dat" in file:
                files_with_ref_path.append(file_path)

    resources_file_path = []
    #resources_file_path.append("resources/MicrosoftYaqiHei-2.ttf")

    folders_path = []
    folders_path.append(base_path + "iconengines")
    folders_path.append(base_path + "imageformats")
    folders_path.append(base_path + "bearer")
    folders_path.append(base_path + "audio")
    folders_path.append(base_path + "mediaservice")
    folders_path.append(base_path + "platforms")
    folders_path.append(base_path + "playlistformats")
    folders_path.append(base_path + "plugins")
    folders_path.append(base_path + "sdw_plugins")
    folders_path.append(base_path + "styles")
    folders_path.append(base_path + "resources")
    folders_path.append(base_path + "generic")
    folders_path.append(base_path + "tls")
    folders_path.append(base_path + "networkinformation")
    folders_path.append(base_path + "tc_app")
    folders_path.append(base_path + "platforminputcontexts")
    folders_path.append(base_path + "qml")
    folders_path.append(base_path + "qmltooling")
    folders_path.append(base_path + "gr_plugins")
    folders_path.append(base_path + "gr_plugins_client")
    folders_path.append(base_path + "gr_client")
    folders_path.append(base_path + "certs")

    target_path = base_path + "package/packages/com.rgaa.gammaray/data"#+ "gammaray" + target_folder_suffix
    if len(in_target_path) > 0:
        target_path = in_target_path

    if force_update and os.path.exists(target_path):
        shutil.rmtree(target_path)

    if not os.path.exists(target_path):
        os.makedirs(target_path)

    for file in files_with_ref_path:
        file_name = file.split("/")[-1]
        print("copy file {} to {}".format(file_name, target_path + "/" + file_name))
        copyfile(file, target_path + "/" + file_name)

    for file in resources_file_path:
        file_name = file.split("/")[-1]
        print("copy file {} to {}".format(file_name, resources_path + "/" + file_name))
        copy2(file, resources_path + "/" + file_name)

    for folder in folders_path:
        file_name = folder.split("/")[-1]
        print("copy folder {} to {}".format(file_name, folder))
        try:
            copytree(folder, target_path + "/" + file_name)
        except:
            print("3rd libs folder already exists, use : force-update if you want to update them.")

# python install.py release/debug/rel-debug
# gen publish folder at : out/build/{x64-debug/x64-release/x64-rel-debug}/publish
if __name__ == "__main__":
    # param 1. release / debug
    #
    print("arg : {}".format(sys.argv))
    force_update = True
    publish = False
    compile_type = sys.argv[1]
    if len(sys.argv) >= 2:
        if sys.argv[1] == "force-update":
            force_update = True
        if sys.argv[1] == "pub-rel-debug":
            force_update = True
            publish = True
            compile_type = "rel-debug"
        if sys.argv[1] == "publish":
            force_update = True
            publish = True
            compile_type = "release"
    target_path = ""
    if len(sys.argv) >= 3:
        target_path = sys.argv[2]

    collceion_program_files(compile_type, force_update, publish, target_path)