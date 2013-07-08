if(${USEQT} STREQUAL "4")

ADD_CUSTOM_TARGET(cutes-qt4.1.gz ALL DEPENDS cutes.man COMMAND gzip -c cutes.man > cutes-qt4.1.gz)
install(FILES cutes-qt4.1.gz DESTINATION share/man/man1)

elseif(${USEQT} STREQUAL "5")

ADD_CUSTOM_TARGET(cutes-qt5.1.gz ALL DEPENDS cutes.man COMMAND gzip -c cutes.man > cutes-qt5.1.gz)
install(FILES cutes-qt5.1.gz DESTINATION share/man/man1)

endif(${USEQT} STREQUAL "4")
