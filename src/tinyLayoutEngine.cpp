#include "tinyLayoutEngine.hpp"

#include <vector>
#include <string>

namespace TinyLayoutEngine {

BaseElement::BaseElement() {

    width = LengthNone;
    height = LengthNone;
    maxWidth = LengthNone;
    maxHeight = LengthNone;
    minWidth = LengthNone;
    minHeight = LengthNone;
    
    paddingLeft = 0;
    paddingRight = 0;
    paddingTop = 0;
    paddingBottom = 0;
    marginLeft = 0;
    marginRight = 0;
    marginTop = 0;
    marginBottom = 0;

    grow = 0; 
    zIndex = 0;

    backgroundColor = {0, 0, 0, 0}; // Default transparent

    borderColor = {0, 0, 0, 0}; // Default transparent
    borderWidth = 0;
    borderRadius = 0;

    positioning = PositionFree;
    alignSelf = AlignStretch;

    visible = true; 
    displayed = true; 
}



Container::Container(){
    elementType = ElementTypeContainer;

    gap = 0; 

    overflow = OverflowGrow;

    layoutDirection = LayoutRow;
    justifyContent = JustifyStart;
    alignItems = AlignStretch;
    
}

Text::Text() {
    elementType = ElementTypeText;

    text = "";
    color = {0, 0, 0}; // Default black
    textAlign = TextAlignLeft;
    font = 0; // Default font
}

Polygon::Polygon() {
    elementType = ElementTypePolygon;

    fill = false; 
    stroke = true; 
}

//Helper function to split a string by whitespace

bool isSpace(char ch) {
    switch (ch) {
        case ' ': case '\t': case '\n': case '\r': case '\v': case '\f': return true;
        default: return false;
    }
}   

std::vector<std::string> splitStringByWhitesp(std::string& s) {
    std::vector<std::string> out;
    const char* p   = s.data();
    const char* end = p + s.size();

    // (Optional) tiny heuristic reserve to reduce vector growth
    out.reserve(8);

    while (p < end) {
        while (p < end && isSpace(static_cast<unsigned char>(*p))) ++p;
        const char* start = p;
        while (p < end && !isSpace(static_cast<unsigned char>(*p))) ++p;
        if (start < p) out.emplace_back(start, static_cast<size_t>(p - start)); // one copy per token
    }

    return out;
}


//
//zero out all widths and heights recursively 
//

void initElements(BaseElement* element){
    
    element->layout.width = 0;
    element->layout.height = 0;

    if(element->elementType == ElementTypeContainer){
        Container* container = (Container*)element;
        for(int i = 0; i < container->children.size(); i++){
            initElements(container->children[i]);
        }
    }
}

//
//First pass, fit sizing, width using reverse breadth first pass. 
//

//compute the fit sizing for parents. 
void computeWidthFitSizing(BaseElement* element, BaseMeasurementContext* measurementContext){

    //go over children first if this element is a container
    if(element->elementType == ElementTypeContainer){
        Container* container = (Container*)element;
        for(int i = 0; i < container->children.size(); i++){
            computeWidthFitSizing(container->children[i], measurementContext);
        }
    }

    //grab element settings related to the width
    int16_t pl = element->paddingLeft;
    int16_t pr = element->paddingRight;
    int16_t blw = element->borderWidth;
    int16_t brw = element->borderWidth;
    int16_t sumSpace = pl + pr + blw + brw;

    //add the padding and boarder to the width
    element->layout.width = sumSpace;
    element->layout.minWidth = sumSpace;

    if(element->elementType == ElementTypeContainer){
        Container* container = (Container*)element;

        //now add the children's widths if the width is auto else just use the set width
        if(element->width >= 0) { 
            element->layout.width = element->width; 
        }
        else if(container->children.size() > 0) {

            LayoutDirection layoutDirection = container->layoutDirection;

            //if the layout is row then add up all the children's widths + gaps
            if(layoutDirection == LayoutRow){
                for(int i = 0; i < container->children.size(); i++){
                    BaseElement* child = container->children[i];
                    element->layout.width += child->layout.width;
                }
                element->layout.width += (container->children.size() -1) * container->gap; 
            }

            //if column get the max width of the children
            else {

                int16_t maxChildWidth = 0;
                for(int i = 0; i < container->children.size(); i++){
                    BaseElement* child = container->children[i];
                    maxChildWidth = (std::max)(maxChildWidth, child->layout.width);
                }
                element->layout.width += maxChildWidth;
            }
        }

        //now add the children's min widths
        if(element->minWidth >= 0) {
            element->layout.minWidth = element->minWidth;
        }
        else if(container->children.size() > 0) {

            LayoutDirection layoutDirection = container->layoutDirection;

            //if the layout is row then add up all the children's widths + gaps
            if(layoutDirection == LayoutRow){
                for(int i = 0; i < container->children.size(); i++){
                    BaseElement* child = container->children[i];
                    element->layout.minWidth += child->layout.minWidth;
                }
                element->layout.minWidth += (container->children.size() -1) * container->gap; 
            }

            //if column get the max width of the children
            else {
                int16_t maxChildMinWidth = 0;
                for(int i = 0; i < container->children.size(); i++){
                    BaseElement* child = container->children[i];
                    maxChildMinWidth = (std::max)(maxChildMinWidth, child->layout.minWidth);
                }
                element->layout.minWidth += maxChildMinWidth;
            }
        }

    }

    else if(element->elementType == ElementTypeText){ 

        Text* textElement = (Text*)element;
        
        //Grab text settings for the elements
        uint8_t font = textElement->font;
        std::string text = textElement->text;
        
        //compute the width which is the natural fully expanded size of the text
        if(element->width >= 0) {
            element->layout.width = element->width;
        }
        else{
            element->layout.width += measurementContext->measureTextWidth(text, font);   
        }
        
        //Compute the min width which is the longest word in the text
        if(element->minWidth >= 0) {
            element->layout.minWidth = element->minWidth;
        }
        else {
            // Minimum width is the longest word
            std::vector<std::string> textSplit = splitStringByWhitesp(text);
            int16_t minSize = 0;    

            for(int i = 0; i < textSplit.size(); i++) {
                std::string word = textSplit[i];
                int16_t wordWidth = measurementContext->measureTextWidth(word, font);
                minSize = (std::max)(minSize, wordWidth);
            }
            
            element->layout.minWidth += minSize;
        }

    }

}

//
//Second pass, Grow and shrink the widths. 
//

//function for growing and shrinking widths
void computeWidthsGrowSizing(Container* parent){

    //grab the paraents layout directions 
    LayoutDirection layoutDirection = parent->layoutDirection;

    //grab element settings related to the width
    int16_t pl = parent->paddingLeft;
    int16_t pr = parent->paddingRight;
    int16_t blw = parent->borderWidth;
    int16_t brw = parent->borderWidth;

    //Compute the initial value for available width
    int16_t availableWidth = parent->layout.width;
    availableWidth -= pl + pr + blw + brw;

    int16_t remainingWidth = availableWidth;
    int16_t remainingMinWidth = availableWidth;

    //for row layouts, we distribute the remaining width among the children based on their flex grow
    if(layoutDirection == LayoutRow){

        //compute the remaining width after subtracting all children's widths and the gaps
        int childCount = parent->children.size();
        int16_t gap = parent->gap;
        if(childCount > 0) {
            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                remainingWidth -= child->layout.width;
                remainingMinWidth -= child->layout.minWidth;
                if(i < childCount -1){
                    remainingWidth -= gap; 
                    remainingMinWidth -= gap;
                }
            }
        }
            
        //if there is remaining width we distribute it
        if(remainingWidth >= 0){

            //compute the sum of the children's grow factors 
            int16_t flexGrowTotal = 0;
            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                int16_t childFlexGrow = child->grow;
                flexGrowTotal += childFlexGrow;
            }  

            //distribute remaining width based on flex grow
            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                int16_t childFlexGrow = child->grow;
                if(flexGrowTotal > 0 && childFlexGrow > 0) {
                    int16_t remainingSpaceProportion = (remainingWidth * childFlexGrow) / flexGrowTotal;
                    child->layout.width += remainingSpaceProportion;
                }
            }
        }

        //if there is no remaining width we distribute the remaining width based on content grow
        else if(remainingMinWidth >= 0){
            
            int16_t contentGrowTotal = 0;
            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                int16_t contentGrow = child->layout.width > (child->layout.minWidth) ? 1 : 0;
                contentGrowTotal += contentGrow;
            }   

            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                int16_t childContentGrow = child->layout.width > (child->layout.minWidth) ? 1 : 0;
                if(contentGrowTotal > 0 && childContentGrow > 0) {
                    int16_t remainingSpaceProportion = (remainingMinWidth * childContentGrow) / contentGrowTotal;
                    child->layout.width = child->layout.minWidth + remainingSpaceProportion;
                }
            }
        }

        //if no remaining width, we shrink children proportionally
        else {

            int16_t totalMinWidth = 0; 
            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                totalMinWidth += child->layout.minWidth;
            }

            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                int16_t childMinWidth = child->layout.minWidth;
                int16_t computedSize = (availableWidth * childMinWidth) / totalMinWidth;
                child->layout.width = computedSize;
            }
        }
    }

    //For column layout, if the parent has the stretch alignment we increase the children's widths to fill the available space
    else {
        
        Alignment parentAlignItems = parent->alignItems;

        int childCount = parent->children.size();
        for(int i = 0; i < childCount; i++) {
            BaseElement* child = parent->children[i];

            if(parentAlignItems == AlignStretch && (child->alignSelf == AlignAuto) || child->alignSelf == AlignStretch) {
                child->layout.width = availableWidth;
            }

            else if(availableWidth < child->layout.width ){
                child->layout.width = availableWidth;
            }                        
        }
    }

    //recur on children
    int childCount = parent->children.size();
    for(int i = 0; i < childCount; i++) {
        BaseElement* child = parent->children[i];

        if(child->elementType == ElementTypeContainer) {
            Container* childAsContainer = (Container*)child;
            computeWidthsGrowSizing(childAsContainer);
        }
    }

} 

//
//Third pass, Wrap the text. 
//

void computeTextWrapping(BaseElement* element, BaseMeasurementContext* measurementContext){
    
    if(element->elementType == ElementTypeText){

        Text* textElement = (Text*)element;

        //Grab the text data
        std::string text = textElement->text;
        uint8_t font = textElement->font;
        int16_t width = textElement->layout.width;
        int16_t pl = element->paddingLeft; 
        int16_t pr = element->paddingRight;
        int16_t bw = element->borderWidth;
        int16_t availableWidth = width - pl - pr - bw - bw; 

        //Compute the wrapped lines for the text in the accessible width
        textElement->wrappedText.clear();
        
        std::vector<std::string> words = splitStringByWhitesp(text);
        std::string currentLine = "";
        int16_t currentLineWidth = 0;

        for(size_t i = 0; i < words.size(); i++){
            std::string word = words[i];
            
            // Measure the width of the word (plus space if not first word)
            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
            int16_t testLineWidth = measurementContext->measureTextWidth(testLine, font);
            
            // If adding this word exceeds available width and current line is not empty
            if(testLineWidth > availableWidth && !currentLine.empty()){
                // Store the current line offset
                textElement->wrappedText.push_back(currentLine);
                
                // Start new line with current word
                currentLine = word;
                currentLineWidth = measurementContext->measureTextWidth(word, font);
            } else {
                // Add word to current line
                currentLine = testLine;
                currentLineWidth = testLineWidth;
            }
        }
        
        // Add the last line if it's not empty
        if(!currentLine.empty()){
            textElement->wrappedText.push_back(currentLine);
        }
        
        // Handle edge case where no words fit (single word too wide)
        if(textElement->wrappedText.empty() && !words.empty()){
            // Force at least one line, even if it overflows
            textElement->wrappedText.push_back(words[0]);
        }
    }

    //recur on children if this element is a container
    if(element->elementType == ElementTypeContainer){
        Container* container = (Container*)element;
        for(int i = 0; i < container->children.size(); i++){
            computeTextWrapping(container->children[i], measurementContext);
        }
    }
}

//
//Forth pass, fit sizing, height using breadth first pass.
//

//compute the fit sizing for parents. 
void computeHeightFitSizing(BaseElement* element, BaseMeasurementContext* measurementContext){

    //go over children first if this element is a container
    if(element->elementType == ElementTypeContainer){
        Container* container = (Container*)element;
        for(int i = 0; i < container->children.size(); i++){
            computeWidthFitSizing(container->children[i], measurementContext);
        }
    }

    //grab element settings related to the width
    int16_t pt = element->paddingTop;
    int16_t pb = element->paddingBottom;
    int16_t btw = element->borderWidth;
    int16_t bbw = element->borderWidth;
    int16_t sumSpace = pt + pb + btw + bbw;

    //add the padding and boarder to the width
    element->layout.height = sumSpace;
    element->layout.minHeight = sumSpace;

    if(element->elementType == ElementTypeContainer){
        Container* container = (Container*)element;

        //now add the children's heights if the height is auto else just use the set height 
        if(element->height >= 0) { 
            element->layout.height = element->height; 
        }
        else if(container->children.size() > 0) {

            LayoutDirection layoutDirection = container->layoutDirection;

            //if the layout is column then add up all the children's heights + gaps
            if(layoutDirection == LayoutColumn){
                for(int i = 0; i < container->children.size(); i++){
                    BaseElement* child = container->children[i];
                    element->layout.height += child->layout.height;
                }
                element->layout.height += (container->children.size() -1) * container->gap; 
            }

            //if column get the max height of the children
            else {

                int16_t maxChildHeight = 0;
                for(int i = 0; i < container->children.size(); i++){
                    BaseElement* child = container->children[i];
                    maxChildHeight = (std::max)(maxChildHeight, child->layout.height);
                }
                element->layout.height += maxChildHeight;
            }
        }


        //now add the children's min heights
        if(element->minHeight >= 0) { 
            element->layout.minHeight = element->minHeight; 
        }
        else if(container->children.size() > 0) {

            LayoutDirection layoutDirection = container->layoutDirection;

            //if the layout is column then add up all the children's heights + gaps
            if(layoutDirection == LayoutColumn){
                for(int i = 0; i < container->children.size(); i++){
                    BaseElement* child = container->children[i];
                    element->layout.minHeight += child->layout.minHeight;
                }
                element->layout.minHeight += (container->children.size() -1) * container->gap; 
            }

            //if column get the max height of the children
            else {

                int16_t maxChildMinHeight = 0;
                for(int i = 0; i < container->children.size(); i++){
                    BaseElement* child = container->children[i];
                    maxChildMinHeight = (std::max)(maxChildMinHeight, child->layout.minHeight);
                }
                element->layout.minHeight += maxChildMinHeight;
            }
        }
    }

    else if(element->elementType == ElementTypeText) {
        
        Text* textElement = (Text*)element;
        
        //Grab text settings for the elements
        uint8_t font = textElement->font;
        std::string text = textElement->text;
        int16_t lineSpacing = 1; //TODO: make this a property of the text element
        
        //compute the height which is the natural line height of the text
        if(element->height >= 0) {
            element->layout.height += element->height;
        }
        else{
            int16_t contentH = 0; 
            for(int i = 0; i < textElement->wrappedText.size(); i++) {
                contentH += measurementContext->getLineHeight(lineSpacing, font);
            }
            element->layout.height += contentH;
        }
        
        //Compute the min height which is just the height of the text I think...
        if(element->minHeight >= 0) {
            element->layout.minHeight += element->minHeight;
        }
        else {
            element->layout.minHeight = element->layout.height; 
        }

    }
}

//
//Fifth pass, Grow and shrink the heights.
//

//function for growing and shrinking heights
void computeHeightsGrowSizing(Container* parent){

    //grab the paraents layout directions 
    LayoutDirection layoutDirection = parent->layoutDirection;
    
    //grab element settings related to the height
    int16_t pt = parent->paddingTop;
    int16_t pb = parent->paddingBottom;
    int16_t btw = parent->borderWidth;
    int16_t bbw = parent->borderWidth;

    //Compute the initial value for available width
    int16_t availableHeight = parent->layout.height;
    availableHeight -= pt + pb + btw + bbw;

    int16_t remainingHeight = availableHeight;
    int16_t remainingMinHeight = availableHeight;

    //for column layouts, we distribute the remaining height among the children based on their flex grow
    if(layoutDirection == LayoutColumn){

        //compute the remaining height after subtracting all children's heights and the gaps
        int childCount = parent->children.size();
        int16_t gap = parent->gap;
        for(int i = 0; i < childCount; i++) {
            BaseElement* child = parent->children[i];
            remainingHeight -= child->layout.height;
            remainingMinHeight -= child->layout.minHeight;
            if(i < childCount -1){
                remainingHeight -= gap; 
                remainingMinHeight -= gap;
            }
        }
        
        
        //if there is remaining height we distribute it
        if(remainingHeight >= 0){

            //compute the sum of the children's grow factors 
            int16_t flexGrowTotal = 0;
            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                int16_t childFlexGrow = child->grow;
                flexGrowTotal += childFlexGrow;
            }  

            //distribute remaining height based on flex grow
            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                int16_t childFlexGrow = child->grow;
                if(flexGrowTotal > 0 && childFlexGrow > 0) {
                    int16_t remainingSpaceProportion = (remainingHeight * childFlexGrow) / flexGrowTotal;
                    child->layout.height += remainingSpaceProportion;
                }
            }
        }

        //if there is no remaining height we distribute the remaining height based on content grow
        else if(remainingMinHeight >= 0){
            
            int16_t contentGrowTotal = 0;
            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                int16_t contentGrow = child->layout.height > (child->layout.minHeight) ? 1 : 0;
                contentGrowTotal += contentGrow;
            }   

            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                int16_t childContentGrow = child->layout.height > (child->layout.minHeight) ? 1 : 0;
                if(contentGrowTotal > 0 && childContentGrow > 0) {
                    int16_t remainingSpaceProportion = (remainingMinHeight * childContentGrow) / contentGrowTotal;
                    child->layout.height = child->layout.minHeight + remainingSpaceProportion;
                }
            }
        }

        //if no remaining height, we shrink children proportionally
        else {

            int16_t totalMinHeight = 0; 
            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                totalMinHeight += child->layout.minHeight;
            }

            for(int i = 0; i < childCount; i++) {
                BaseElement* child = parent->children[i];
                int16_t childMinHeight = child->layout.minHeight;
                int16_t computedSize = (availableHeight * childMinHeight) / totalMinHeight;
                child->layout.height = computedSize;
            }
        }
    }

    //For column layout, if the parent has the stretch alignment we increase the children's height to fill the available space
    else {
        
        Alignment parentAlignItems = parent->alignItems;

        int childCount = parent->children.size();
        for(int i = 0; i < childCount; i++) {
            BaseElement* child = parent->children[i];

            if(parentAlignItems == AlignStretch && (child->alignSelf == AlignAuto) || child->alignSelf == AlignStretch) {
                child->layout.height = availableHeight;
            }

            else if(availableHeight < child->layout.height ){
                child->layout.height = availableHeight;
            }                        
        }
    }

    //recur on children
    int childCount = parent->children.size();
    for(int i = 0; i < childCount; i++) {
        BaseElement* child = parent->children[i];
        if(child->elementType == ElementTypeContainer) {
            Container* childAsContainer = (Container*)child;
            computeHeightsGrowSizing(childAsContainer);
        }
    }
} 


//
//Sixth pass, Position elements
//

void computePositions(Container* parent){

    //grab the paraents layout directions 
    LayoutDirection layoutDirection = parent->layoutDirection;

    //grab parent data 
    int16_t pl = parent->paddingLeft;
    int16_t pr = parent->paddingRight;
    int16_t blw = parent->borderWidth;
    int16_t brw = parent->borderWidth;
    int16_t x = parent->layout.x;
    int16_t width = parent->layout.width;

    int16_t pt = parent->paddingTop;
    int16_t pb = parent->paddingBottom;
    int16_t btw = parent->borderWidth;
    int16_t bbw = parent->borderWidth;
    int16_t y = parent->layout.y;
    int16_t height = parent->layout.height;

    Alignment parentAlignItems = parent->alignItems;
    
    int16_t gap = parent->gap;

    int childCount = parent->children.size();

    //position elements horizontally
    int16_t currentHOffset = 0;
    
    if(layoutDirection == LayoutRow) {
        for(int i = 0; i < childCount; i++) {
            BaseElement* child = parent->children[i];
            child->layout.x = x + pl + blw + currentHOffset;
            currentHOffset += child->layout.width + gap; 
        }
    }
    else {
        for(int i = 0; i < childCount; i++) {
            BaseElement* child = parent->children[i];
            child->layout.x = x + pl + blw;

            if(parentAlignItems == AlignCenter && !child->alignSelf || child->alignSelf == AlignCenter){
                child->layout.x += (width - pl - blw - pr - brw - child->layout.width) / 2;
            }
            else if(parentAlignItems == AlignEnd && !child->alignSelf || child->alignSelf == AlignEnd){
                child->layout.x += (width - pl - blw - pr - brw - child->layout.width);
            }
        }
    }

    //position elements vertically
    int16_t currentVOffset = 0;
    
    if(layoutDirection == LayoutColumn) {
        for(int i = 0; i < childCount; i++) {
            BaseElement* child = parent->children[i];
            child->layout.y = y + pt + btw + currentVOffset;
            currentVOffset += child->layout.height + gap; 
        }
    }
    else {
        for(int i = 0; i < childCount; i++) {
            BaseElement* child = parent->children[i];
            child->layout.y = y + pt + btw;

            if(parentAlignItems == AlignCenter && !child->alignSelf || child->alignSelf == AlignCenter){
                child->layout.y += (height - pt - btw - pb - bbw - child->layout.height) / 2;
            }
            else if(parentAlignItems == AlignEnd && !child->alignSelf || child->alignSelf == AlignEnd){
                child->layout.y += (height - pt - btw - pb - bbw - child->layout.height);
            }
        }
    }

    //recur on children
    childCount = parent->children.size();
    for(int i = 0; i < childCount; i++) {
        BaseElement* child = parent->children[i];
        if(child->elementType == ElementTypeContainer) {
            Container* childAsContainer = (Container*)child;
            computePositions(childAsContainer);
        }
    }
}

//This is the main function that does the layout stuff
void layout(Container* container, BaseMeasurementContext* measurementContext) {
    initElements(container);
    computeWidthFitSizing(container, measurementContext);
    computeWidthsGrowSizing(container);
    computeTextWrapping(container, measurementContext);
    computeHeightFitSizing(container, measurementContext);
    computeHeightsGrowSizing(container);
    computePositions(container);
}


} // namespace TinyLayoutEngine
