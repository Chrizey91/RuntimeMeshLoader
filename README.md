RuntimeMeshLoader
==================
A pluging for Unreal Engine 4 and Unreal Engine 5, which allows to import meshes during runtime.

This is a fork of the original [RuntimeMeshLoader repository](https://github.com/GameInstitute/RuntimeMeshLoader).

Additionally, functiones were added to import and apply textures for the meshes.

RuntimeMeshLoader (RML) uses [Assimp](https://github.com/assimp/assimp) as a third-party library to handle most of the import-stuff.
 
## Table of contents
* [Supported operating systems](#supported-operating-systems)
* [Supported Unreal Enginge versions](#supported-unreal-enginge-versions)
* [Installation](#installation)
* [How-to](#how-to)
* [Limitations](#limitations)


## Supported operating systems
Currently, Windows (x64) as well as Linux (Ubuntu 20) have been verified.

## Supported Unreal Enginge versions
The Plugin was tested under Unreal Engine v. 4.25.3 as well as version 5 (Early Access). However, any modern version of Unreal Engine should be compatible with it.

## Installation
Download this repository, by clicking on the green `Code`-Button in the upper right hand corner and select `Download ZIP`.

In your Unreal Enginge project open the `Plugins` directory. If it does not exist, simply create a new folder in the root directory of your project and call it `Plugins`.

Copy the contents of the former downloaded ZIP-folder into it. The folder might have a name such as `RuntimeMeshLoader-main`. If that is the case, rename it to `RuntimeMeshLoader`.

Afterwards, load up your project. Unreal Enginge might tell you it needs to rebuild the Plugin. Simply click on `Yes`. After the rebuild process your project should load as usual.

In your projec's `source panel` you should now have a folder labeled `RuntimeMeshLoader Content`. Inside you will find an example map, which will show you how to use the plugin with an example.

## How-to
The plugin can be used in many ways. You could create a file-dialogue for the user to select his assets to be loaded, or by loading assets from a pre-defined folder, or any other creative way.

In the example map, a folder called `RuntimeMeshLoader` is created in the user's `Documents` folder. Inside this folder, the plugin expects for each asset to be loaded a separate folder, in which 3 files should be located. `<folder name>.fbx`, `<folder name>_T.png` and `<folder name>_N.png`, which are the mesh itself as an fbx-file, its texture and its normal map as PNG-files.

If you load up the plugin for the first time, the folder will be empty. As an example, you can copy the two provided mesh-folders inside the `Resources` directory into the aforementioned `RuntimeMeshLoader`-folder.

## Limitations
Only PNG-formats can be loaded. JPG is currently not supported.

In the example map, only fbx-files are supported. However, the plugin is already capable of laoding a lot more 3D-file formats.

I often run into problems with loading fbx-files downloaded from the internet. Usually, opening it up in Blender and exporting it again solved it for me.
