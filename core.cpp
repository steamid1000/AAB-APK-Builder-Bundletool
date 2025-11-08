#include <systemtopologyapi.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <future>

// 7Zip is a dependency of this program,please install it.
// Java is also a dependency of this program, get that too

const std::string bundleToolFilePath = "../resources/bundletool-all-1.15.6.jar";
const std::string tempBundlefileName = "Temp-out.apks";

class Spinner
{
    std::atomic<bool> running{true};
    std::thread spinnerThread;

    void spin()
    {
        const char *spinners = "|/-\\";
        int i = 0;

        while (running)
        {
            std::cout << "\r" << spinners[i++ % 4] << "Processing...";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        // clear the loading spinner
        std::cout << "\r" << std::string(30, ' ') << "\r";
    }

public:
    void start()
    {
        running = true;
        spinnerThread = std::thread(&Spinner::spin, this);
    }

    void stop()
    {
        running = false;
        if (spinnerThread.joinable())
        {
            spinnerThread.join();
        }
    }
};

std::string fileName(std::string bundleFilePath)
// This function return the name of the aab file so that we can use it for the final apk
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

void takePathsandKeys(std::string &bundleFilePath, std::string &keystoreFilePath, std::string &keyPass, std::string &keyAlias, std::string &aliasPassword, std::string &finalExtractName)
{
    // get the aab file path
    if (bundleFilePath.empty())
    {
        printf("Paste the .aab file path along with its name: \n");
        std::cin >> bundleFilePath;
    }

    if (keystoreFilePath.empty())
    {
        // get the keystore file path otherwise we will fall back to the default keystore
        printf("Past the keystore path for signing this apk(or type no to use the default keystore): \n");
        std::cin >> keystoreFilePath;
    }

    // extract the name of the aab file using the method above
    finalExtractName = fileName(bundleFilePath);

    // get the passwords of the keystores
    if (keyPass.empty())
    {
        printf("Enter the keystore password: \n");
        std::cin >> keyPass;
    }

    if (keyAlias.empty())
    {
        printf("Enter the key alias name : \n");
        std::cin >> keyAlias;
    }

    if (aliasPassword.empty())
    {
        printf("Enter the alias password: \n");
        std::cin >> aliasPassword;
    }
}

std::string makeZipBuildCommand(const std::string &bundleFilePath, const std::string &keystoreFilePath, const std::string &keyPass, const std::string &keyAlias, const std::string &aliasPassword, const std::string &tempBundlefileName, const std::string &dotApks)
{
    // this function will create the command to build the apk zip file using bundletool
    // java -jar bundletool-all-1.15.6.jar build-apks --bundle=path\to\app.aab --output=path\to\output.apks --ks=path\to\keystore.jks --ks-pass=pass:your_keystore_password --ks-key-alias=your_key_alias --key-pass=pass:your_key_password --mode=universal

    // Now we have all the required variables,lets build the command
    return "java -jar " + bundleToolFilePath + " build-apks --mode=universal --bundle=" + bundleFilePath + " --output=" + tempBundlefileName + dotApks + " --ks=" + keystoreFilePath + " --ks-key-alias=" +
           keyAlias + " --ks-pass=pass:" + keyPass + " --key-pass=pass:" + aliasPassword;
}

void doPostProcessing(std::string &finalExtractName, const std::string &tempBundlefileName, const std::string &dotApks, const std::string &dotzip)
{
    printf("\n\n\t\tZip Built..\n");

    // Lets rename it
    std::string rename = "rename " + tempBundlefileName + dotApks + " " + tempBundlefileName + dotzip;
    const char *renameCommand = rename.c_str();
    system(renameCommand);
    printf("\t\tFile Renamed Successfully..\n");

    // Now Lets Extract the apk file from the zip file using 7zip cmd
    std::string extract = "7z x " + tempBundlefileName + dotzip + " *.apk";
    const char *extractCommand = extract.c_str();
    system(extractCommand);
    printf("\t\tApk Extracted..\n");

    // Now lets remove the temp zip file as we no longer need it
    std::string deleteTemp = "del " + tempBundlefileName + dotzip;
    const char *deleteTempCommand = deleteTemp.c_str();
    system(deleteTempCommand);
    printf("\t\tDeleted Temp files..\n");

    // Now lets rename the apk to the .aab file name
    rename = "rename Universal.apk " + finalExtractName + ".apk";
    const char *apkRename = rename.c_str();
    system(apkRename);

    printf("\t\tApk File Renamed..\n");

    printf("\t\tProcess: Success\n");
}

int main(int argc, char *argv[])
{

    // First argument will always be the file name,so we skip it.
    // Step 1: get the path of the file
    // Step 2: run the bundletool command
    // Step 3: rename the .apks to .7zip and extract it
    // Step 3: extract the created apk
    // Note: we can take the package name as the final apk name

    // spinner
    Spinner loadSpinner;

    std::string bundleFilePath;
    int buildResult = -1;

    // keystore variables
    std::string keystoreFilePath;
    std::string keyPass;
    std::string keyAlias;
    std::string aliasPassword;

    // extensions and name variables
    std::string finalExtractName;
    std::string dotApks = ".apks";
    std::string dotzip = ".zip";

    // check which values were passed as arguments to the program
    // positions: bundlePath -> keystorePath -> keyStorePass -> storeAlias -> aliasPass
    if (argc > 1 && argc == 6)
    {
        bundleFilePath = argv[1];
        keystoreFilePath = argv[2];
        keyPass = argv[3];
        keyAlias = argv[4];
        aliasPassword = argv[5];
    }
    else if (argc > 1 && argc <= 6) // this means that some of the arugments were missing, so we need to take input of them
    {
        for (size_t i = 1; i < argc; i++)
        {
            switch (i)
            {
            case 1:
                bundleFilePath = argv[i];
                break;
            case 2:
                keystoreFilePath = argv[i];
                break;
            case 3:
                keyPass = argv[i];
                break;
            case 4:
                keyAlias = argv[i];
                break;
            case 5:
                aliasPassword = argv[i];
                break;
            default:
                break;
            }
        }

        printf("\n Some arguments are still missing, fill them out.\n");
    }

    // take the required paths and passwords from the user
    takePathsandKeys(bundleFilePath, keystoreFilePath, keyPass, keyAlias, aliasPassword, finalExtractName);

    // make the zip build command
    std::string buildCommand = makeZipBuildCommand(bundleFilePath, keystoreFilePath, keyPass, keyAlias, aliasPassword, tempBundlefileName, dotApks);

    // run this command, the sytem function requires a const char*
    std::future<int> buildResultFuture = std::async(std::launch::async, [&]()
                                                    {
        const char *finalbuildCommand = buildCommand.c_str();
        finalbuildCommand; // here we are making a apk zip file
        return system(finalbuildCommand); });

    loadSpinner.start();
    buildResult = buildResultFuture.get();
    loadSpinner.stop();

    // check if the command was successful
    if (buildResult == 0)
    {
        doPostProcessing(finalExtractName, tempBundlefileName, dotApks, dotzip);
    }
    else
    {
        printf("Command failed with code: %i \n", buildResult);
    }

    return EXIT_SUCCESS;
}