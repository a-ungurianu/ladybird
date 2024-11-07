/*
 * Copyright (c) 2021-2024, Sam Atkins <sam@ladybird.org>
 * Copyright (c) 2023, Tim Flynn <trflynn89@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/String.h>
#include <LibJS/Runtime/VM.h>
#include <LibWeb/CSS/CSS.h>
#include <LibWeb/CSS/Parser/Parser.h>
#include <LibWeb/CSS/Parser/PropertySyntaxParser.h>
#include <LibWeb/CSS/PropertyID.h>
#include <LibWeb/CSS/PropertyName.h>
#include <LibWeb/CSS/Serialize.h>
#include <LibWeb/HTML/Window.h>

namespace Web::CSS {

// https://www.w3.org/TR/cssom-1/#dom-css-escape
WebIDL::ExceptionOr<String> escape(JS::VM&, StringView identifier)
{
    // The escape(ident) operation must return the result of invoking serialize an identifier of ident.
    return serialize_an_identifier(identifier);
}

// https://www.w3.org/TR/css-conditional-3/#dom-css-supports
bool supports(JS::VM& vm, StringView property, StringView value)
{
    auto& realm = *vm.current_realm();

    // 1. If property is an ASCII case-insensitive match for any defined CSS property that the UA supports,
    //    and value successfully parses according to that property’s grammar, return true.
    if (auto property_id = property_id_from_string(property); property_id.has_value()) {
        if (parse_css_value(Parser::ParsingContext { realm }, value, property_id.value()))
            return true;
    }

    // 2. Otherwise, if property is a custom property name string, return true.
    else if (is_a_custom_property_name_string(property)) {
        return true;
    }

    // 3. Otherwise, return false.
    return false;
}

// https://www.w3.org/TR/css-conditional-3/#dom-css-supports
WebIDL::ExceptionOr<bool> supports(JS::VM& vm, StringView condition_text)
{
    auto& realm = *vm.current_realm();

    // 1. If conditionText, parsed and evaluated as a <supports-condition>, would return true, return true.
    if (auto supports = parse_css_supports(Parser::ParsingContext { realm }, condition_text); supports && supports->matches())
        return true;

    // 2. Otherwise, If conditionText, wrapped in parentheses and then parsed and evaluated as a <supports-condition>, would return true, return true.
    auto wrapped_condition_text = TRY_OR_THROW_OOM(vm, String::formatted("({})", condition_text));

    if (auto supports = parse_css_supports(Parser::ParsingContext { realm }, wrapped_condition_text); supports && supports->matches())
        return true;

    // 3. Otherwise, return false.
    return false;
}

// https://www.w3.org/TR/css-properties-values-api-1/#the-registerproperty-function
WebIDL::ExceptionOr<void> register_property(JS::VM& vm, PropertyDefinition const& property_def)
{
    // To register a custom property with name being a string, and optionally syntax being a string,
    // inherits being a boolean, and initialValue being a string, execute these steps:

    auto& realm = *vm.current_realm();

    // 1. Let *property set* be the value of the current global object’s associated Document’s [[registeredPropertySet]] slot.
    auto& window = verify_cast<HTML::Window>(HTML::current_principal_global_object());
    auto& property_set = window.associated_document().registered_property_set();

    // 2. If name is not a custom property name string, throw a SyntaxError and exit this algorithm.
    if (!is_a_custom_property_name_string(property_def.name)) {
        return WebIDL::SyntaxError::create(realm, "Bad property name"_string);
    }

    //    If property set already contains an entry with name as its property name (compared codepoint-wise),
    //    throw an InvalidModificationError and exit this algorithm.

    // TODO: Does this count as codepoint-wise comparison?
    if (property_set.contains(property_def.name)) {
        return WebIDL::InvalidModificationError::create(realm, "Property already registed"_string);
    }

    // 3. Attempt to consume a syntax definition from syntax. If it returns failure, throw a SyntaxError.
    //    Otherwise, let syntax definition be the returned syntax definition.
    Parser::PropertySyntaxParser syntax_parser(property_def.syntax);

    auto syntax = syntax_parser.parse_syntax();
    if (syntax.is_error()) {
        return WebIDL::SyntaxError::create(realm, "Invalid syntax string"_string);
    }

    // 4. If syntax definition is the universal syntax definition, and initialValue is not present, let parsed initial value be empty.
    //    This must be treated identically to the "default" initial value of custom properties, as defined in [css-variables].
    //    Skip to the next step of this algorithm.

    //    Otherwise, if syntax definition is the universal syntax definition, parse initialValue as a <declaration-value>. If this fails, throw a SyntaxError and exit this algorithm. Otherwise, let parsed initial value be the parsed result. Skip to the next step of this algorithm.

    //    Otherwise, if initialValue is not present, throw a SyntaxError and exit this algorithm.

    //    Otherwise, parse initialValue according to syntax definition. If this fails, throw a SyntaxError and exit this algorithm.

    //    Otherwise, let parsed initial value be the parsed result. If parsed initial value is not computationally independent, throw a SyntaxError and exit this algorithm.

    // 5. Set inherit flag to the value of inherits.

    // 6. Let registered property be a struct with
    //    a property name of name,
    //    a syntax of syntax definition,
    //    an initial value of parsed initial value, and
    //    an inherit flag of inherit flag.
    //    Append registered property to property set.

    property_set.set(property_def.name, {});
    return {};
}

}
