import qbs
Project
{
    name: "DTools"
    references: [
        "DTLib/DTLib.qbs"
    ]


SubProject {
        filePath: "test/DTTest.qbs"
}

}
