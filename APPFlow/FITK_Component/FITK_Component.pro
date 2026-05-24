TEMPLATE = subdirs

SUBDIRS += \
    $$PWD/FITKGeoCompOCC \
    $$PWD/FITKRenderWindowVTK \
    $$PWD/FITKMeshGenOF \
    $$PWD/FITKWidget \
    $$PWD/FITKCompMessageWidget \
    #$$PWD/FITKRenderWindowOCC \
    $$PWD/FITKFluidVTKGraphAdaptor \
    $$PWD/FITKOFDictWriter \
    $$PWD/FITKOFDriver \
    $$PWD/FITKOFMeshIO \
#    $$PWD/FITKPlotWindow \
    $$PWD/FITKGeoOCCIOHDF5 \
    $$PWD/FITKFlowOFIOHDF5 \

CONFIG += ordered
