
class QPixmap;

namespace Bespin {
namespace Elements {

    BLIB_EXPORT void setShadowIntensity(float intensity);
    BLIB_EXPORT void setScale(float scale);
    BLIB_EXPORT QPixmap glow(int size, float width);
    BLIB_EXPORT QPixmap shadow(int size, bool opaque, bool sunken, float factor = 1.0);
    BLIB_EXPORT QPixmap roundMask(int size);
    BLIB_EXPORT QPixmap roundedMask(int size, int factor);
    BLIB_EXPORT QPixmap sunkenShadow(int size, bool enabled);
    BLIB_EXPORT QPixmap relief(int size, bool enabled);
    BLIB_EXPORT QPixmap groupShadow(int size);

#if 0
BLIB_EXPORT void renderButtonLight(Tile::Set &set);
BLIB_EXPORT void renderLightLine(Tile::Line &line);
#endif

}
}

#undef fillRect
