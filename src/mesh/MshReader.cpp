#include "MshReader.h"

#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QTextStream>

namespace
{
bool readRequiredLine(QTextStream &stream, QString &line, QString *errorMessage, const QString &context)
{
    if (stream.atEnd()) {
        if (errorMessage) {
            *errorMessage = "Unexpected end of file while reading " + context + ".";
        }
        return false;
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
}

bool MshReader::readMsh2(const QString &filePath, MeshData &meshData, QString *errorMessage)
{
    if (!QFileInfo::exists(filePath)) {
        if (errorMessage) {
            *errorMessage = "MSH file does not exist: " + filePath;
        }
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = "Failed to open MSH file: " + file.errorString();
        }
        return false;
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
            QString formatLine;
            if (!readRequiredLine(stream, formatLine, errorMessage, "$MeshFormat")) {
                return false;
            }

            const QStringList parts = formatLine.split(' ', Qt::SkipEmptyParts);
            if (parts.size() < 3) {
                if (errorMessage) {
                    *errorMessage = "Invalid $MeshFormat line.";
                }
                return false;
            }

            double version = 0.0;
            int fileType = -1;
            if (!parseDouble(parts.at(0), version) || !parseInt(parts.at(1), fileType)) {
                if (errorMessage) {
                    *errorMessage = "Invalid $MeshFormat values.";
                }
                return false;
            }
            if (version < 2.0 || version >= 3.0 || fileType != 0) {
                if (errorMessage) {
                    *errorMessage = "Only Gmsh msh2 ASCII files are supported.";
                }
                return false;
            }

            QString endLine;
            if (!readRequiredLine(stream, endLine, errorMessage, "$EndMeshFormat")) {
                return false;
            }
            if (endLine != "$EndMeshFormat") {
                if (errorMessage) {
                    *errorMessage = "Expected $EndMeshFormat.";
                }
                return false;
            }
            hasMeshFormat = true;
        } else if (section == "$Nodes") {
            QString countLine;
            if (!readRequiredLine(stream, countLine, errorMessage, "$Nodes count")) {
                return false;
            }

            int nodeCount = 0;
            if (!parseInt(countLine, nodeCount) || nodeCount < 0) {
                if (errorMessage) {
                    *errorMessage = "Invalid node count in $Nodes section.";
                }
                return false;
            }

            loadedMesh.nodes.reserve(static_cast<size_t>(nodeCount));
            for (int index = 0; index < nodeCount; ++index) {
                QString nodeLine;
                if (!readRequiredLine(stream, nodeLine, errorMessage, "$Nodes data")) {
                    return false;
                }

                const QStringList parts = nodeLine.split(' ', Qt::SkipEmptyParts);
                if (parts.size() != 4) {
                    if (errorMessage) {
                        *errorMessage = "Invalid node line: " + nodeLine;
                    }
                    return false;
                }

                MeshNode node;
                if (!parseInt(parts.at(0), node.id)
                        || !parseDouble(parts.at(1), node.x)
                        || !parseDouble(parts.at(2), node.y)
                        || !parseDouble(parts.at(3), node.z)) {
                    if (errorMessage) {
                        *errorMessage = "Invalid node values: " + nodeLine;
                    }
                    return false;
                }
                loadedMesh.nodes.push_back(node);
            }

            QString endLine;
            if (!readRequiredLine(stream, endLine, errorMessage, "$EndNodes")) {
                return false;
            }
            if (endLine != "$EndNodes") {
                if (errorMessage) {
                    *errorMessage = "Expected $EndNodes.";
                }
                return false;
            }
            hasNodes = true;
        } else if (section == "$Elements") {
            QString countLine;
            if (!readRequiredLine(stream, countLine, errorMessage, "$Elements count")) {
                return false;
            }

            int elementCount = 0;
            if (!parseInt(countLine, elementCount) || elementCount < 0) {
                if (errorMessage) {
                    *errorMessage = "Invalid element count in $Elements section.";
                }
                return false;
            }

            for (int index = 0; index < elementCount; ++index) {
                QString elementLine;
                if (!readRequiredLine(stream, elementLine, errorMessage, "$Elements data")) {
                    return false;
                }

                const QStringList parts = elementLine.split(' ', Qt::SkipEmptyParts);
                if (parts.size() < 3) {
                    if (errorMessage) {
                        *errorMessage = "Invalid element line: " + elementLine;
                    }
                    return false;
                }

                int elementId = 0;
                int elementType = 0;
                int tagCount = 0;
                if (!parseInt(parts.at(0), elementId)
                        || !parseInt(parts.at(1), elementType)
                        || !parseInt(parts.at(2), tagCount)
                        || tagCount < 0) {
                    if (errorMessage) {
                        *errorMessage = "Invalid element values: " + elementLine;
                    }
                    return false;
                }

                const int nodeStart = 3 + tagCount;
                if (elementType != 4) {
                    continue;
                }
                if (parts.size() < nodeStart + 4) {
                    if (errorMessage) {
                        *errorMessage = "Invalid tetrahedron element line: " + elementLine;
                    }
                    return false;
                }

                TetraElement tetra;
                tetra.id = elementId;
                if (!parseInt(parts.at(nodeStart), tetra.node1)
                        || !parseInt(parts.at(nodeStart + 1), tetra.node2)
                        || !parseInt(parts.at(nodeStart + 2), tetra.node3)
                        || !parseInt(parts.at(nodeStart + 3), tetra.node4)) {
                    if (errorMessage) {
                        *errorMessage = "Invalid tetrahedron node ids: " + elementLine;
                    }
                    return false;
                }
                loadedMesh.tetraElements.push_back(tetra);
            }

            QString endLine;
            if (!readRequiredLine(stream, endLine, errorMessage, "$EndElements")) {
                return false;
            }
            if (endLine != "$EndElements") {
                if (errorMessage) {
                    *errorMessage = "Expected $EndElements.";
                }
                return false;
            }
            hasElements = true;
        }
    }

    if (!hasMeshFormat) {
        if (errorMessage) {
            *errorMessage = "Missing $MeshFormat section.";
        }
        return false;
    }
    if (!hasNodes) {
        if (errorMessage) {
            *errorMessage = "Missing $Nodes section.";
        }
        return false;
    }
    if (!hasElements) {
        if (errorMessage) {
            *errorMessage = "Missing $Elements section.";
        }
        return false;
    }
    if (loadedMesh.nodes.empty()) {
        if (errorMessage) {
            *errorMessage = "MSH file contains no nodes.";
        }
        return false;
    }
    if (loadedMesh.tetraElements.empty()) {
        if (errorMessage) {
            *errorMessage = "MSH file contains no 4-node tetrahedron elements.";
        }
        return false;
    }

    meshData = std::move(loadedMesh);
    return true;
}
