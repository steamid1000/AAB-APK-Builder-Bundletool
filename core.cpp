#include <stdio.h>
#include <systemtopologyapi.h>
#include <string>

// 7Zip is a dependency of this program,please install it.
// Java is also a dependency of this program, get that too

std::string fileName(std::string bundleFilePath) // This function return the name of the aab file so that we can use it for the final apk
{
    // We will find the . in the .aab file path name and go backwards until be encounter a '/' or a '\' or a '\\'
    short dotPos = bundleFilePath.find('.');
    short temp = dotPos;
    while (bundleFilePath[temp] != '/')
    {
        temp--;
        if (bundleFilePath[temp] == '\\')
        {
            break;
        }
    }

    std::string name = bundleFilePath.substr(temp + 1, dotPos - temp - 1);
    return name;
}

int main()
{

    // First argument will always be the file name,so we skip it.

    // Step 1: get the path of the file
    // Step 2: run the bundletool command
    // Step 3: rename the .apks to .7zip and extract it
    // Step 3: extract the created apk

    // Note: we can take the package name as the final apk name

    std::string bundleToolFilePath = "resources/bundletool-all-1.15.6.jar";
    std::string tempBundlefileName = "Temp-out.apks";
    std::string bundleFilePath;
    std::string finalExtractName;
    std::string dotApks = ".apks";
    std::string dotzip = ".zip";

    printf( "Paste the .aab file path along with its name: \n");
    char* tmp = new char[50];
    scanf("%s",tmp);
    bundleFilePath = tmp;
    finalExtractName = fileName(bundleFilePath);
    std::string buildCommand = "java -jar " + bundleToolFilePath + " build-apks --mode=universal --bundle=" + bundleFilePath + " --output=" + tempBundlefileName + dotApks;
    const char *finalbuildCommand = buildCommand.c_str();
    system(finalbuildCommand);
    printf( "Done..\n");

    // Lets rename it
    std::string rename = "rename " + tempBundlefileName + dotApks + " " + tempBundlefileName + dotzip;
    const char *renameCommand = rename.c_str();
    system(renameCommand);
    printf( " File Renamed Successfully..\n");

    // Now Lets Extract the apk file from the zip file using 7zip cmd
    std::string extract = "7z x " + tempBundlefileName + dotzip + " *.apk";
    const char *extractCommand = extract.c_str();
    system(extractCommand);
    printf( "Apk Extracted..\n");

    // Now lets remove the temp zip file as we no longer need it
    std::string deleteTemp = "del " + tempBundlefileName+dotzip;
    const char* deleteTempCommand = deleteTemp.c_str(); 
    system(deleteTempCommand);
    printf("Deleted Temp files..\n");

    // Now lets rename the apk to the .aab file name
    rename = "rename Universal.apk " + finalExtractName + ".apk";
    const char *apkRename = rename.c_str();
    system(apkRename);

    printf( "Apk File Renamed..\n");
    printf( "Process: Success\n");

    return 0;
}