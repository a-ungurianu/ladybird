#include <AK/Error.h>
#include <AK/String.h>

namespace Web::CSS::Parser {

// TODO: This is the same as tokenizer. It should share somehow
static inline bool is_low_line(u32 code_point)
{
    return code_point == 0x5F;
}

class PropertySyntaxComponent {

public:
    enum class Multiplier {
        None,
        SpaceSeparated,
        CommaSeparated
    };

    enum class Type {
        DataType,
        Ident
    };

    PropertySyntaxComponent(StringView name, Type type, Multiplier multiplier)
        : m_name(name)
        , m_type(type)
        , m_multiplier(multiplier)
    {
    }
    StringView const& name()
    {
        return m_name;
    }
    Multiplier multiplier()
    {
        return m_multiplier;
    }
    Type type()
    {

        return m_type;
    }

private:
    StringView m_name;
    Type m_type;
    Multiplier m_multiplier;
};

class PropertySyntax {
    enum Type {
        Universal,
        Composite
    };

private:
    PropertySyntax() = default;

public:
    static PropertySyntax create_universal_syntax();

    static PropertySyntax create_composite_syntax(AK::Vector<PropertySyntaxComponent>&& components);

private:
    Type m_type;
    Vector<PropertySyntaxComponent> m_components;
};

class PropertySyntaxParser {
public:
    PropertySyntaxParser(StringView syntax_string)
        : m_syntax_string(syntax_string)
    {
    }

    // https://www.w3.org/TR/css-properties-values-api-1/#consume-syntax-definition
    ErrorOr<PropertySyntax> parse_syntax();

private:
    ErrorOr<PropertySyntaxComponent> consume_a_syntax_component();

    ErrorOr<StringView> consume_a_data_type_name();
    ErrorOr<StringView> consume_an_ident_sequence();

    bool is_custom_ident(StringView ident);

    StringView m_syntax_string;
};
};
