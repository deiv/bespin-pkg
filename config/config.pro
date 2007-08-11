TEMPLATE = subdirs
if ( $$(CONFIG_QT) ) {
   SUBDIRS += bespin-config.pro
}
if ( $$(CONFIG_KDE) ) {
   SUBDIRS += kstyle-bespin-config.pro
}