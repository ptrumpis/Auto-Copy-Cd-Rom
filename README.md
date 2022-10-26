# Auto Copy CD-Rom
Automatically copy all contents of inserted CD/DVDs to a pre-defined destination in your Windows filesystem

### Introduction
I wrote this C++ project in 2012 with Visual Studio to have a ton of DVDs automatically copied to a network storage.
I did some small updates in 2022 to make the old code compile with JetBrains CLion.

I no longer have any use for this project and the code is horribly formatted for my personal and modern standards.

It is however still a good code example how to access CD/DVD drives in Windows applications. It can be easily modified to work with any media devices like removable USB sticks for example.

### About
The program scans all your existing CD/DVD drives and will wait until a media gets inserted.
It will then start a new thread to copy all the contents to a location that was specified in a configuration file. See below.

### Configuration

The following example .ini file is included in the project, it's name must remain unchanged and it must be in the same folder as the binary .exe file when you run the application.

*File: AutoCopyCdRom.ini*
```ini
[default]
Path="C:\CDCopy\default"
SubFolder=1

[Example]
Label="BACKUPDISC"
Path="C:\CDCopy\BackUpDisc"
```
It has a `[default]` section which defines where all content should be copied to by default.
You can change the `Path` key to your own folder destination.

The `SubFolder` key can be set to `0` or `1`.

If the flag is set to `1` the content of the media will be placed inside a sub folder that matches the name of the media label.
The value `0` disables this behavior.

###### Copying known CD/DVDs to a different location

You can create multiple custom sections with CD/DVD labels.
If an inserted media matches the `Label` key, it will use the `Path` of your custom section instead the default one.

*Let's say you have a special backup disc that you want to copy to a special back up folder.*

This behavior is shown in the `[Example]` section of the configuration file.
