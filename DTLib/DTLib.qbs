import qbs
import qbs.Environment
import qbs.FileInfo

StaticLibrary {
    name: "DTLib"
    Group
    {
        name: "Sourcefiles"
        prefix: "src/"
    files: [
            "PriorityMutex.cpp",
            "SharedPriorityMutex.cpp"
        ]
    }

    Group
    {
        name: "HeaderFiles"
        prefix: "include/"
    files: [
            "MiscTools.h",
            "PriorityMutex.h",
            "Singleton.h",
            "SynchronizedValue.h"
        ]
    }

    version: "1.0.0"
    install: true

    Depends { name: 'cpp' }

    cpp.includePaths:[
        "include/"
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [exportingProduct.sourceDirectory, FileInfo.joinPaths(exportingProduct.sourceDirectory, "/include/"), FileInfo.joinPaths(exportingProduct.sourceDirectory, "/src/")]
        cpp.cxxLanguageVersion: ["c++23"]
    }
    cpp.cxxLanguageVersion: ["c++23"]
}
