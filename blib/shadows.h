
namespace Bespin {
namespace Shadows {
    enum BLIB_EXPORT Type { None = 0, Small, Large };
    BLIB_EXPORT void cleanUp();
    BLIB_EXPORT void set(WId id, Shadows::Type t, bool storeToRoot = false);
} }

