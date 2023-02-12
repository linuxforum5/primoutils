rm *.wav
SLIDES=""
for FN in `ls zx` ; do
    echo "$FN"
    convert zx/$FN -crop 512x384+64+48 tmp.png
    convert tmp.png  -resize 256x192  tmp2.png
    convert tmp2.png -threshold 50% MONO:p/$FN.raw
    convert tmp2.png -threshold 50% gif/$FN.gif
    SLIDES="$SLIDES -R p/$FN.raw -R p/$FN.raw"
done
slideshow $SLIDES -R p/end.png.raw -o slideshow.wav

# convert -monochrome -colors 2 -geometry 256x192 img1.png img1_mono.gif

#convert -size 2560x1920 xc:white -colors 2 c2560x1920.gif
# convert c2560x1920.gif -resize 256x192 -threshold 50% img1.xbm

# convert $SRC -resize 256x192 -threshold 50% img1.xbm
# 512x384
# convert $SRC -resize 256x192 -threshold 50% -monochrome -depth 1 MONO:test1.scr
# convert $SRC -resize 256x192 -threshold 50% -monochrome -depth 1 test1.gif

##convert img1.png -resize 256x192 -threshold 50% img1_mono.gif
# convert -alpha extract -compress none img1_mono.gif img1.h
##convert -compress none img1_mono.gif img1.h

#convert img1.png -resize 256x192 -compress none -threshold 50% img1.h
#convert img1.png -resize 256x192 -compress none -colors 2 img1m.bmp
#convert img1m.bmp -define h:format=gray -depth 1 -size 256x192 -compress none img1.h
# convert img1m.bmp -define h:format=MONO -compress none img1.h
# PBM PNM

#convert img1.png -resize 256x192 -define h:format=MONO img1.h
#convert img1.png -colors 2 -resize 256x192 MONO img1.h

# convert img1m.bmp -define h:format=XBM -compress none img1.h

#convert -size 256x192 xc:white -colors 2 c256x192.gif
#convert c256x192.gif -define h:format=XBM -compress none -colors 2 img1.h

# convert c256x192.gif c256x192.xbm
#convert c256x192.gif img1_mono.h
