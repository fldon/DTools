StaticLibrary {
    name: "DTools"
    Group
    {
        name: "Sourcefiles"
        prefix: "src/"
    files: [
            "PriorityMutex.cpp"
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
        cpp.includePaths: [exportingProduct.sourceDirectory, exportingProduct.includePaths]
    }
}
