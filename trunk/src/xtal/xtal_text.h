
#pragma once

namespace xtal{

AnyPtr text(const StringPtr& text);

AnyPtr format(const StringPtr& text);

void set_text_map(const MapPtr& map);

void add_text_map(const MapPtr& map);

MapPtr get_text_map();

}
