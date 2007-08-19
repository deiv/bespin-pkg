TEMPLATE = subdirs
if ( $$(CONFIG_KDE) ) {
   SUBDIRS += kstyle-bespin-config.pro
}
if ( $$(CONFIG_QT) ) {
   SUBDIRS += bespin.pro
}
