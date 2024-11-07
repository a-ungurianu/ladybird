#include <LibWeb/CSS/Parser/PropertySyntaxParser.h>

namespace Web::CSS::Parser {

ErrorOr<PropertySyntaxComponent> PropertySyntaxParser::consume_a_syntax_component()
{
    // To consume a syntax component from a stream of code points stream:
    //     Consume as much whitespace as possible from stream.
    m_syntax_string = m_syntax_string.trim_whitespace(TrimMode::Left);
    //     Let component be a new syntax component with its name and multiplier initially empty.
    StringView name;
    PropertySyntaxComponent::Type type;
    PropertySyntaxComponent::Multiplier multiplier = PropertySyntaxComponent::Multiplier::None;
    //     Consume the next input code point in stream:

    //          U+003C LESS-THAN SIGN (<)
    //          Consume a data type name from stream.
    if (m_syntax_string.starts_with('<')) {
        //      If it returned a string, set component’s name to the returned value. Otherwise, return failure.
        name = TRY(consume_a_data_type_name());
        type = PropertySyntaxComponent::Type::DataType;
    }
    //          ident-start code point
    //          U+005C REVERSE SOLIDUS (\)
    else if (!m_syntax_string.is_empty() && (is_ascii_alpha(m_syntax_string[0]) || !is_ascii(m_syntax_string[0]) || is_low_line(m_syntax_string[0]))) {
        //          If the stream starts with an ident sequence, reconsume the current input code point from stream then consume an ident sequence from stream, and set component’s name to the returned value.
        //          Otherwise return failure.
        name = TRY(consume_an_ident_sequence());
        type = PropertySyntaxComponent::Type::Ident;

        //          If component’s name does not parse as a <custom-ident>, return failure.
        if (!is_custom_ident(name)) {
            return Error::from_string_literal("Invalid custom identifier");
        }
    } else {
        //          anything else
        //          Return failure.
        return Error::from_string_literal("Unexpected start of syntax component");
    }

    // If component’s name is a pre-multiplied data type name, return component.
    if (name == "transform-list" && type == PropertySyntaxComponent::Type::DataType) {
        name = "transform-function"sv;
        multiplier = PropertySyntaxComponent::Multiplier::SpaceSeparated;
        return PropertySyntaxComponent { name, type, multiplier };
    }

    if (!m_syntax_string.is_empty()) {
        if (m_syntax_string.starts_with('+')) {
            m_syntax_string = m_syntax_string.substring_view(1);
            multiplier = PropertySyntaxComponent::Multiplier::SpaceSeparated;
        } else if (m_syntax_string.starts_with('#')) {
            m_syntax_string = m_syntax_string.substring_view(1);
            multiplier = PropertySyntaxComponent::Multiplier::CommaSeparated;
        }
    }

    return PropertySyntaxComponent(name, type, multiplier);
}

PropertySyntax PropertySyntax::create_universal_syntax()
{
    PropertySyntax r {};
    r.m_type = Type::Universal;
    return r;
}
PropertySyntax PropertySyntax::create_composite_syntax(AK::Vector<PropertySyntaxComponent>&& components)
{
    PropertySyntax r {};
    r.m_type = Type::Composite;
    r.m_components = std::move(components);
    return r;
}
ErrorOr<PropertySyntax> PropertySyntaxParser::parse_syntax()
{
    // This section describes how to consume a syntax definition from a string `string`.
    // It either produces a syntax definition with a list of syntax components, or the universal syntax definition.

    // 1. Strip leading and trailing ASCII whitespace from string.
    m_syntax_string = m_syntax_string.trim_whitespace();

    // 2. If string’s length is 0, return failure.
    if (m_syntax_string.is_empty()) {
        return Error::from_string_literal("Property syntax can't be an empty string");
    }
    // 3. If string’s length is 1, and the only code point in string is U+002A ASTERISK (*),
    //    return the universal syntax definition.

    if (m_syntax_string == "*") {
        return PropertySyntax::create_universal_syntax();
    }

    // 4. Let stream be an input stream created from the code points of string, preprocessed as specified in [css-syntax-3].
    // Let definition be an initially empty list of syntax components.
    Vector<PropertySyntaxComponent> definition;

    do {
        // 5. Consume a syntax component from stream.
        //    If failure was returned, return failure;
        auto component = TRY(consume_a_syntax_component());

        //    otherwise, append the returned value to definition.
        definition.append(component);

        //    Consume as much whitespace as possible from stream.
        m_syntax_string = m_syntax_string.trim_whitespace(TrimMode::Left);

        //    Consume the next input code point in stream:

        //    EOF                      -> return definition.
        //    U+007C VERTICAL LINE (|) -> Repeat step 5.
    } while (m_syntax_string.starts_with('|'));

    if (m_syntax_string.is_empty()) {
        return PropertySyntax::create_composite_syntax(std::move(definition));
    }
    //    Anything else            -> Return failure.
    return Error::from_string_literal("Invalid property syntax string");
}
ErrorOr<StringView> PropertySyntaxParser::consume_a_data_type_name()
{
    // We assume this starts with `<`
    size_t pos = 1;
    TODO();
}
ErrorOr<StringView> PropertySyntaxParser::consume_an_ident_sequence()
{
    TODO();
}
bool PropertySyntaxParser::is_custom_ident(StringView ident)
{
    (void)ident;
    TODO();
}
}
