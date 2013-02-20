ADD_CUSTOM_TARGET(cutes.1.gz ALL DEPENDS cutes.man COMMAND gzip -c cutes.man > cutes.1.gz)
install(FILES cutes.1.gz DESTINATION share/man/man1)
