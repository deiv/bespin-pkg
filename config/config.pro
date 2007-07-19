TEMPLATE = subdirs
eval($$(CONFIG_QT) = 1) {
   SUBDIRS += bespin-config.pro
}
eval($$(CONFIG_KDE) = 1) {
   SUBDIRS += kstyle-bespin-config.pro
}