
#pragma once

namespace xtal{

AnyPtr text(const StringPtr& text);

AnyPtr format(const StringPtr& text);

void assign_text_map(const AnyPtr& map_iter);

void append_text_map(const AnyPtr& map_iter);

}
