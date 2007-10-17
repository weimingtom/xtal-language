
#pragma once

namespace xtal{

class Int : public Any{};
class Float : public Any{};
class True : public Any{};
class False : public Any{};


class Range : public Base{
public:

	Range(const AnyPtr& begin, const AnyPtr& end, bool exclude = false);

public:

	const AnyPtr& begin(){
		return begin_;
	}

	const AnyPtr& end(){
		return end_;
	}

	AnyPtr each();

private:

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & begin_ & end_;
	}

	AnyPtr begin_;
	AnyPtr end_;

};

}
