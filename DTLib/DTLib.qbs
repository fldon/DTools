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
            "../include/concurrency/Thread_Pool.cpp",
            "concurrency/PriorityMutex.cpp",
            "concurrency/SharedPriorityMutex.cpp",
            "geometry/geometry.cpp",
        ]
    }

    Group
    {
        name: "HeaderFiles"
        prefix: "include/"
    files: [
            "MiscTools.h",
            "concurrency/Misc_Conc.h",
            "concurrency/PriorityMutex.h",
            "Singleton.h",
            "concurrency/Synch_Stack.h",
            "concurrency/Synch_Value.h",
            "concurrency/Thread_Pool.h",
            "debug.h",
            "concurrency/synch_queue.h",
            "geometry/geometry.h",
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
        cpp.dynamicLibraries: ["stdc++exp"]
    }
    cpp.cxxLanguageVersion: ["c++23"]

    Properties{
        condition: qbs.buildVariant == "debug"
        cpp.defines: ["DT_DEBUG"]
    }
}
