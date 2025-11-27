#include <cstdint>
#include <string>
#include <vector>

#include <emscripten/bind.h>

#include "tinyLayoutEngine.hpp"

using namespace emscripten;
using namespace TinyLayoutEngine;

// Wrapper: only inherit from wrapper<BaseMeasurementContext>
// wrapper<BaseMeasurementContext> already derives from BaseMeasurementContext.
class BaseMeasurementContextWrapper : public wrapper<BaseMeasurementContext> {
public:
    EMSCRIPTEN_WRAPPER(BaseMeasurementContextWrapper);

    int16_t measureTextWidth(std::string& str, uint8_t font) override {
        // Call the JS method "measureTextWidth(str, font)"
        return call<int16_t>("measureTextWidth", str, font);
    }

    int16_t getLineHeight(int16_t lineSpacing, uint8_t font) override {
        // Call the JS method "getLineHeight(lineSpacing, font)"
        return call<int16_t>("getLineHeight", lineSpacing, font);
    }
};

EMSCRIPTEN_BINDINGS(TinyLayoutEngine_bindings) {

    //
    // Enums
    //
    enum_<ElementType>("ElementType")
        .value("ElementTypeContainer", ElementTypeContainer)
        .value("ElementTypeText",      ElementTypeText)
        .value("ElementTypePolygon",   ElementTypePolygon);

    enum_<Positioning>("Positioning")
        .value("PositionFree",   PositionFree)
        .value("PositionLayout", PositionLayout);

    enum_<Alignment>("Alignment")
        .value("AlignStretch",      AlignStretch)
        .value("AlignStart",        AlignStart)
        .value("AlignEnd",          AlignEnd)
        .value("AlignCenter",       AlignCenter)
        .value("AlignSpaceBetween", AlignSpaceBetween)
        .value("AlignAuto",         AlignAuto);

    enum_<Justification>("Justification")
        .value("JustifyStart",  JustifyStart)
        .value("JustifyEnd",    JustifyEnd)
        .value("JustifyCenter", JustifyCenter);

    enum_<LayoutDirection>("LayoutDirection")
        .value("LayoutRow",    LayoutRow)
        .value("LayoutColumn", LayoutColumn);

    enum_<OverflowMode>("OverflowMode")
        .value("OverflowGrow",   OverflowGrow)
        .value("OverflowHide",   OverflowHide)
        .value("OverflowScroll", OverflowScroll);

    enum_<ComputedLength>("ComputedLength")
        .value("LengthAuto", LengthAuto)
        .value("LengthNone", LengthNone)
        .value("LengthFull", LengthFull);

    enum_<TextAlignment>("TextAlignment")
        .value("TextAlignLeft",    TextAlignLeft)
        .value("TextAlignRight",   TextAlignRight)
        .value("TextAlignCenter",  TextAlignCenter)
        .value("TextAlignJustify", TextAlignJustify);

    //
    // Plain structs
    //
    value_object<Color>("Color")
        .field("r", &Color::r)
        .field("g", &Color::g)
        .field("b", &Color::b)
        .field("a", &Color::a);

    value_object<ComputedLayout>("ComputedLayout")
        .field("x",        &ComputedLayout::x)
        .field("y",        &ComputedLayout::y)
        .field("minWidth", &ComputedLayout::minWidth)
        .field("minHeight",&ComputedLayout::minHeight)
        .field("width",    &ComputedLayout::width)
        .field("height",   &ComputedLayout::height);

    //
    // std::vector registrations
    //
    register_vector<BaseElement*>("BaseElementPtrVector");
    register_vector<std::string>("StringVector");
    register_vector<int16_t>("Int16Vector");

    //
    // BaseElement
    //
    class_<BaseElement>("BaseElement")
        .constructor<>()
        .property("layout",        &BaseElement::layout)

        .property("borderWidth",   &BaseElement::borderWidth)
        .property("borderRadius",  &BaseElement::borderRadius)

        .property("width",         &BaseElement::width)
        .property("height",        &BaseElement::height)
        .property("maxWidth",      &BaseElement::maxWidth)
        .property("maxHeight",     &BaseElement::maxHeight)
        .property("minWidth",      &BaseElement::minWidth)
        .property("minHeight",     &BaseElement::minHeight)

        .property("paddingLeft",   &BaseElement::paddingLeft)
        .property("paddingRight",  &BaseElement::paddingRight)
        .property("paddingTop",    &BaseElement::paddingTop)
        .property("paddingBottom", &BaseElement::paddingBottom)

        .property("marginLeft",    &BaseElement::marginLeft)
        .property("marginRight",   &BaseElement::marginRight)
        .property("marginTop",     &BaseElement::marginTop)
        .property("marginBottom",  &BaseElement::marginBottom)

        .property("grow",          &BaseElement::grow)
        .property("zIndex",        &BaseElement::zIndex)

        .property("backgroundColor",&BaseElement::backgroundColor)
        .property("borderColor",    &BaseElement::borderColor)

        .property("elementType",   &BaseElement::elementType)
        .property("positioning",   &BaseElement::positioning)
        .property("alignSelf",     &BaseElement::alignSelf)

        .property("visible",       &BaseElement::visible)
        .property("displayed",     &BaseElement::displayed)
        ;

    //
    // Container
    //
    class_<Container, base<BaseElement>>("Container")
        .constructor<>()
        .property("children",      &Container::children)
        .property("gap",           &Container::gap)
        .property("overflow",      &Container::overflow)
        .property("layoutDirection",&Container::layoutDirection)
        .property("justifyContent",&Container::justifyContent)
        .property("alignItems",    &Container::alignItems)
        ;

    //
    // Text
    //
    class_<Text, base<BaseElement>>("Text")
        .constructor<>()
        .property("text",        &Text::text)
        .property("wrappedText", &Text::wrappedText)
        .property("color",       &Text::color)
        .property("textAlign",   &Text::textAlign)
        .property("font",        &Text::font)
        ;

    //
    // Polygon
    //
    class_<Polygon, base<BaseElement>>("Polygon")
        .constructor<>()
        .property("points", &Polygon::points)
        .property("fill",   &Polygon::fill)
        .property("stroke", &Polygon::stroke)
        ;

    //
    // BaseMeasurementContext (virtual, implemented in JS)
    //
    // Note: we do NOT expose .function("measureTextWidth", ...) here to avoid
    // the std::string& binding issue. We only need the virtual dispatch
    // from C++ -> JS, which the wrapper + allow_subclass provide.
    class_<BaseMeasurementContext>("BaseMeasurementContext")
        .allow_subclass<BaseMeasurementContextWrapper>("BaseMeasurementContextWrapper");

    //
    // Free function: layout
    //
    function("layout", &layout, allow_raw_pointers());
}
