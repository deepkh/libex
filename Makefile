
$(RUNTIME)/bin/fdkaac.$(DLLSUFFIX): libfdkaac
	$(MAKE) -C $< install; \
	$(MAKE) -C $< clean;

$(RUNTIME)/bin/x264.$(DLLSUFFIX): libx264
	$(MAKE) -C $< install; \
	$(MAKE) -C $< clean;

$(RUNTIME)/bin/ffmpeg2.$(DLLSUFFIX): libffmpeg2
	$(MAKE) -C $< install; \
	$(MAKE) -C $< clean;

