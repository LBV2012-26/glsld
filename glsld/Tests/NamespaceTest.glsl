int Read();

namespace A
{
    using Vector = vec4;
    const float Scale = 2.0;

    float F(Vector v)
    {
        return dot(v, v);
    }

    namespace Inner
    {
        using Scalar = float;
        Scalar Read()
        {
            return Scale;
        }

        auto r = ::Read();
    }
}

void Between()
{
}

namespace B
{
    int F(int v)
    {
        return v;
    }
}

namespace A
{
    float G(Vector v)
    {
        return F(v) * Inner::Read();
    }
}

namespace A
{
    using Scalar = float;
    const Scalar value = 2.0;
}

namespace A::B::C
{
    const auto x = value;
}

namespace A::B::C
{
    const auto y = x + value;

    struct MyStruct
    {
        int data;
    };
}

void main()
{
    auto v = A::Vector(1.0);

    auto a = A::F(v);                // float
    auto b = B::F(1);                // int
    auto c = A::G(v);                // float
    auto d = (A::Inner::Scalar)1;    // float
    const auto e = ::A::Scale + 1.0; // 可求值为 3

    auto f = (A::Scalar)1;
    auto g = A::B::C::y;

    namespace Abc = A::B::C;
    auto ms = Abc::MyStruct(10);
    auto md = ms.data;
}