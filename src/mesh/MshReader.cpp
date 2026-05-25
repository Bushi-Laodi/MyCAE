#include "MshReader.h"

#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QTextStream>

namespace
{
bool fail(QString *errorMessage, const QString &message)
{
    if (errorMessage) {
        *errorMessage = message;
    }
    return false;
}

bool readRequiredLine(QTextStream &stream, QString &line, QString *errorMessage, const QString &context)
{
    if (stream.atEnd()) {
        return fail(errorMessage, "Unexpected end of file while reading " + context + ".");
    }

    line = stream.readLine().trimmed();
    return true;
}

bool parseInt(const QString &text, int &value)
{
    bool ok = false;
    value = text.toInt(&ok);
    return ok;
}

bool parseDouble(const QString &text, double &value)
{
    bool ok = false;
    value = text.toDouble(&ok);
    return ok;
}

bool expectSectionEnd(QTextStream &stream, const QString &sectionName, QString *errorMessage)
{
    QString endLine;
    if (!readRequiredLine(stream, endLine, errorMessage, sectionName)) {
        return false;
    }
    return endLine == sectionName
        || fail(errorMessage, "Expected " + sectionName + ".");
}

bool readMeshFormatSection(QTextStream &stream, QString *errorMessage)
{
    QString formatLine;
    if (!readRequiredLine(stream, formatLine, errorMessage, "$MeshFormat")) {
        return false;
    }

    const QStringList parts = formatLine.split(' ', Qt::SkipEmptyParts);
    if (parts.size() < 3) {
        return fail(errorMessage, "Invalid $MeshFormat line.");
    }

    double version = 0.0;
    int fileType = -1;
    if (!parseDouble(parts.at(0), version) || !parseInt(parts.at(1), fileType)) {
        return fail(errorMessage, "Invalid $MeshFormat values.");
    }
    if (version < 2.0 || version >= 3.0 || fileType != 0) {
        return fail(errorMessage, "Only Gmsh msh2 ASCII files are supported.");
    }
    return expectSectionEnd(stream, "$EndMeshFormat", errorMessage);
}

bool readPhysicalNamesSection(QTextStream &stream, MeshData &meshData, QString *errorMessage)
{
    QString countLine;
    if (!readRequiredLine(stream, countLine, errorMessage, "$PhysicalNames count")) {
        return false;
    }

    int physicalGroupCount = 0;
    if (!parseInt(countLine, physicalGroupCount) || physicalGroupCount < 0) {
        return fail(errorMessage, "Invalid physical group count in $PhysicalNames section.");
    }

    meshData.physicalGroups.reserve(static_cast<size_t>(physicalGroupCount));
    for (int index = 0; index < physicalGroupCount; ++index) {
        QString physicalGroupLine;
        if (!readRequiredLine(stream, physicalGroupLine, errorMessage, "$PhysicalNames data")) {
            return false;
        }

        const int firstQuote = physicalGroupLine.indexOf('"');
        const int lastQuote = physicalGroupLine.lastIndexOf('"');
        const QString prefix = firstQuote >= 0
            ? physicalGroupLine.left(firstQuote).trimmed()
            : physicalGroupLine;
        const QStringList parts = prefix.split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 2 || firstQuote < 0 || lastQuote <= firstQuote) {
            return fail(errorMessage, "Invalid physical group line: " + physicalGroupLine);
        }

        MeshPhysicalGroup physicalGroup;
        if (!parseInt(parts.at(0), physicalGroup.dimension)
                || !parseInt(parts.at(1), physicalGroup.tag)) {
            return fail(errorMessage, "Invalid physical group values: " + physicalGroupLine);
        }
        physicalGroup.name = physicalGroupLine.mid(firstQuote + 1, lastQuote - firstQuote - 1);
        meshData.physicalGroups.push_back(physicalGroup);
    }

    return expectSectionEnd(stream, "$EndPhysicalNames", errorMessage);
}

bool readNodesSection(QTextStream &stream, MeshData &meshData, QString *errorMessage)
{
    QString countLine;
    if (!readRequiredLine(stream, countLine, errorMessage, "$Nodes count")) {
        return false;
    }

    int nodeCount = 0;
    if (!parseInt(countLine, nodeCount) || nodeCount < 0) {
        return fail(errorMessage, "Invalid node count in $Nodes section.");
    }

    meshData.nodes.reserve(static_cast<size_t>(nodeCount));
    for (int index = 0; index < nodeCount; ++index) {
        QString nodeLine;
        if (!readRequiredLine(stream, nodeLine, errorMessage, "$Nodes data")) {
            return false;
        }

        const QStringList parts = nodeLine.split(' ', Qt::SkipEmptyParts);
        if (parts.size() != 4) {
            return fail(errorMessage, "Invalid node line: " + nodeLine);
        }

        MeshNode node;
        if (!parseInt(parts.at(0), node.id)
                || !parseDouble(parts.at(1), node.x)
                || !parseDouble(parts.at(2), node.y)
                || !parseDouble(parts.at(3), node.z)) {
            return fail(errorMessage, "Invalid node values: " + nodeLine);
        }
        meshData.nodes.push_back(node);
    }

    return expectSectionEnd(stream, "$EndNodes", errorMessage);
}

bool readTriangleElement(const QStringList &parts, int elementId, int tagCount, int nodeStart, MeshData &meshData, QString *errorMessage)
{
    if (parts.size() < nodeStart + 3) {
        return fail(errorMessage, "Invalid triangle element line: " + parts.join(' '));
    }

    SurfaceTriangleElement triangle;
    triangle.id = elementId;
    if (tagCount > 0) {
        parseInt(parts.at(3), triangle.physicalGroupTag);
    }
    if (!parseInt(parts.at(nodeStart), triangle.node1)
            || !parseInt(parts.at(nodeStart + 1), triangle.node2)
            || !parseInt(parts.at(nodeStart + 2), triangle.node3)) {
        return fail(errorMessage, "Invalid triangle node ids: " + parts.join(' '));
    }
    meshData.surfaceTriangles.push_back(triangle);
    return true;
}

bool readTetraElement(const QStringList &parts, int elementId, int nodeStart, MeshData &meshData, QString *errorMessage)
{
    if (parts.size() < nodeStart + 4) {
        return fail(errorMessage, "Invalid tetrahedron element line: " + parts.join(' '));
    }

    TetraElement tetra;
    tetra.id = elementId;
    if (!parseInt(parts.at(nodeStart), tetra.node1)
            || !parseInt(parts.at(nodeStart + 1), tetra.node2)
            || !parseInt(parts.at(nodeStart + 2), tetra.node3)
            || !parseInt(parts.at(nodeStart + 3), tetra.node4)) {
        return fail(errorMessage, "Invalid tetrahedron node ids: " + parts.join(' '));
    }
    meshData.tetraElements.push_back(tetra);
    return true;
}

bool readElementLine(const QString &elementLine, MeshData &meshData, QString *errorMessage)
{
    const QStringList parts = elementLine.split(' ', Qt::SkipEmptyParts);
    if (parts.size() < 3) {
        return fail(errorMessage, "Invalid element line: " + elementLine);
    }

    int elementId = 0;
    int elementType = 0;
    int tagCount = 0;
    if (!parseInt(parts.at(0), elementId)
            || !parseInt(parts.at(1), elementType)
            || !parseInt(parts.at(2), tagCount)
            || tagCount < 0) {
        return fail(errorMessage, "Invalid element values: " + elementLine);
    }

    const int nodeStart = 3 + tagCount;
    if (elementType == 2) {
        return readTriangleElement(parts, elementId, tagCount, nodeStart, meshData, errorMessage);
    }
    if (elementType == 4) {
        return readTetraElement(parts, elementId, nodeStart, meshData, errorMessage);
    }
    return true;
}

bool readElementsSection(QTextStream &stream, MeshData &meshData, QString *errorMessage)
{
    QString countLine;
    if (!readRequiredLine(stream, countLine, errorMessage, "$Elements count")) {
        return false;
    }

    int elementCount = 0;
    if (!parseInt(countLine, elementCount) || elementCount < 0) {
        return fail(errorMessage, "Invalid element count in $Elements section.");
    }

    for (int index = 0; index < elementCount; ++index) {
        QString elementLine;
        if (!readRequiredLine(stream, elementLine, errorMessage, "$Elements data")) {
            return false;
        }
        if (!readElementLine(elementLine, meshData, errorMessage)) {
            return false;
        }
    }

    return expectSectionEnd(stream, "$EndElements", errorMessage);
}

bool validateMesh(const MeshData &meshData, bool hasMeshFormat, bool hasNodes, bool hasElements, QString *errorMessage)
{
    if (!hasMeshFormat) {
        return fail(errorMessage, "Missing $MeshFormat section.");
    }
    if (!hasNodes) {
        return fail(errorMessage, "Missing $Nodes section.");
    }
    if (!hasElements) {
        return fail(errorMessage, "Missing $Elements section.");
    }
    if (meshData.nodes.empty()) {
        return fail(errorMessage, "MSH file contains no nodes.");
    }
    if (meshData.tetraElements.empty()) {
        return fail(errorMessage, "MSH file contains no 4-node tetrahedron elements.");
    }
    return true;
}
}

bool MshReader::readMsh2(const QString &filePath, MeshData &meshData, QString *errorMessage)
{
    if (!QFileInfo::exists(filePath)) {
        return fail(errorMessage, "MSH file does not exist: " + filePath);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return fail(errorMessage, "Failed to open MSH file: " + file.errorString());
    }

    MeshData loadedMesh;
    loadedMesh.name = QFileInfo(filePath).completeBaseName();
    loadedMesh.mshFilePath = QFileInfo(filePath).absoluteFilePath();

    QTextStream stream(&file);
    bool hasMeshFormat = false;
    bool hasNodes = false;
    bool hasElements = false;

    while (!stream.atEnd()) {
        const QString section = stream.readLine().trimmed();
        if (section.isEmpty()) {
            continue;
        }

        if (section == "$MeshFormat") {
            if (!readMeshFormatSection(stream, errorMessage)) {
                return false;
            }
            hasMeshFormat = true;
        } else if (section == "$PhysicalNames") {
            if (!readPhysicalNamesSection(stream, loadedMesh, errorMessage)) {
                return false;
            }
        } else if (section == "$Nodes") {
            if (!readNodesSection(stream, loadedMesh, errorMessage)) {
                return false;
            }
            hasNodes = true;
        } else if (section == "$Elements") {
            if (!readElementsSection(stream, loadedMesh, errorMessage)) {
                return false;
            }
            hasElements = true;
        }
    }

    if (!validateMesh(loadedMesh, hasMeshFormat, hasNodes, hasElements, errorMessage)) {
        return false;
    }

    meshData = std::move(loadedMesh);
    return true;
}
