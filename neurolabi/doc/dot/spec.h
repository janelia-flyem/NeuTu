%include spec2.h
%include spec3.dot

class ParentClass;
class Class1:ParentClass;
class Class2;
class Class3;
stm statement1 = "statement1";
stm statement2 = "statement2";
stm statement3 = "statement3";

function Class1::function;
function Class2::function2;
function Class3::function3;

cmp Class1::Class2;
agg Class3::Class4;

connect Class1::signal, Class2::slot;
connect Class1::signal, Class3::@signal;
connect ZStackDoc, locsegChainSelected, ZStackFrame, setLocsegChainInfo;
connect ZStackDoc, SIGNAL(swcVisibleStateChanged()),
        ZStackView, SLOT(paintObject());


