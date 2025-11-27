#ifndef TINY_LAYOUT_ENGINE_HPP
#define TINY_LAYOUT_ENGINE_HPP

#include <vector>
#include <string>

namespace TinyLayoutEngine {

enum ElementType : int8_t {
    ElementTypeContainer, // A container that can hold other elements
    ElementTypeText, // A text element
    ElementTypePolygon // A polygon element
};

enum Positioning : int8_t {
    PositionFree, // Free positioning, element can be placed anywhere using x, y coordinates
    PositionLayout // Layout positioning, element is positioned based on the layout rules of its container
};

enum Alignment : int8_t{
    AlignStretch, // Stretch the element's box to fill the available space 
    AlignStart, // Align the element's box to the start of the available space
    AlignEnd, // Align the element's box to the end of the available space
    AlignCenter, // Center the element's box in the available space
    AlignSpaceBetween, // Distribute the element's box with space between them in the container
    AlignAuto //let the layout engine decide based on context
};

enum Justification : int8_t{
    JustifyStart, // Justify content to the start of the container
    JustifyEnd, // Justify content to the end of the container
    JustifyCenter // Center content in the container
};

enum LayoutDirection : int8_t{
    LayoutRow, // Layout items in a row (horizontal)
    LayoutColumn // Layout items in a column (vertical)
};

enum OverflowMode : int8_t{
    OverflowGrow, // Grow the container to fit content
    OverflowHide, // Hide content that overflows the container
    OverflowScroll // Allow scrolling for overflowing content
};

enum ComputedLength : int8_t{
    LengthAuto = -1, 
    LengthNone = -2, 
    LengthFull = -3,
};

enum TextAlignment : int8_t{
    TextAlignLeft, 
    TextAlignRight, 
    TextAlignCenter, 
    TextAlignJustify
};

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a; 
};

//This is the computed layout after running the layout function, size of 12 bytes
struct ComputedLayout {
    int16_t x; // Computed x position of the element
    int16_t y; // Computed y position of the element
    int16_t minWidth; // Computed minimum width of the element
    int16_t minHeight; // Computed minimum height of the element
    int16_t width; // Computed width of the element
    int16_t height; // Computed height of the element
}; 

//Base class for all elements, size of 47 bytes
class BaseElement {
public:
    
    ComputedLayout layout; // Computed layout of the element
    
    int16_t borderWidth; // Border width of the container
    int16_t borderRadius; // Border radius for rounded corners
    
    int16_t width; // Width of the element
    int16_t height; // Height of the element
    int16_t maxWidth; // Maximum width of the element
    int16_t maxHeight; // Maximum height of the element
    int16_t minWidth; // Minimum width of the element
    int16_t minHeight; // Minimum height of the element
    
    int16_t paddingLeft; // Padding around the content
    int16_t paddingRight;
    int16_t paddingTop;
    int16_t paddingBottom;

    int16_t marginLeft; // Margin around the element, also used as x and y offset for free positioned elements
    int16_t marginRight; 
    int16_t marginTop; 
    int16_t marginBottom; 

    int8_t grow; // Grow factor for the element in the layout (for flexbox-like behavior)
    int8_t zIndex; // Z-index for stacking order of the element

    Color backgroundColor; // Background color of the container
    Color borderColor; // Border color of the container

    ElementType elementType; // Type of the element (container, text, polygon)
    Positioning positioning; // Positioning of the element in the layout
    Alignment alignSelf; // Self-alignment of the element within its container

    bool visible; // Whether the element is visible (still layouted but not drawn)
    bool displayed; // Whether the element is layouted and takes space in the layout

    BaseElement(); 
}; 


class Container: public BaseElement {
public:
    std::vector<BaseElement*> children; // List of child elements in the container

    int16_t gap; // Gap between elements in a container
    
    OverflowMode overflow; // Overflow behavior of the container

    LayoutDirection layoutDirection; // Direction of the layout (row or column)
    Justification justifyContent; // Justification of the content within the container
    Alignment alignItems; // Alignment of the content within the container

    Container();
}; 

class Text: public BaseElement {
public:
    std::string text; // The text content of the element
    std::vector<std::string> wrappedText; // The text split into lines after wrapping
    Color color; // Color of the text
    TextAlignment textAlign; // Text alignment within it's container
    uint8_t font; //There are a maximum of 256 pre defined fonts. This includes face, size, bold, italic etc.

    Text();
}; 

class Polygon: public BaseElement {
public:
    std::vector<int16_t> points; // List of x, y pairs
    bool fill; // Whether to fill the polygon
    bool stroke; // Whether to draw the stroke

    Polygon(); 
};

//This is the interface that the layout engine uses to measure text and the like. It must be provided. 
class BaseMeasurementContext {
public:

    virtual ~BaseMeasurementContext() = default; // add this

    // Measure the width of the given text with the given font properties
    virtual int16_t measureTextWidth(std::string& str, uint8_t font) = 0;

    //Gets the standard line height of a font, this is (ascent + descent * lineSpacing)
    virtual int16_t getLineHeight(int16_t lineSpacing, uint8_t font) = 0;
};


//This is the main function that does the layout stuff
void layout(Container* container, BaseMeasurementContext* measurementContext);


} // namespace TinyLayoutEngine

#endif // TINY_LAYOUT_ENGINE_HPP