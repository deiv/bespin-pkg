TEMPLATE = subdirs
SUBDIRS = bespin.pro config
unix {
   SUBDIRS += picturepusher/bespinPP.pro
}