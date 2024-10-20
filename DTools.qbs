import qbs
Project
{
    name: "DTools"
    references: [
        "DTLib.qbs"
    ]


SubProject {
        filePath: "test/DTTest.qbs"
}

}
