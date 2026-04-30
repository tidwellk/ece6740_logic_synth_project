# This script converts a BLIF file into a factored Boolean expression,
# then prints the top-level product as one factor per line.
#
# The output format is:
#   - one factor per line
#   - literals in the same product factor are separated by spaces
#
# Example:
#   cd'(a+b)
#
# becomes:
#   c
#   d'
#   a b
#
# This preserves products and sums instead of expanding everything
# into a flat sum-of-products form.
#
# Usage:
#   python3 blif_to_working_format.py input.blif
#   python3 blif_to_working_format.py input.blif output.txt

import sys
from collections import defaultdict

"""
    Read a BLIF file and extract:
      - primary input names
      - primary output names
      - a dictionary mapping each signal name to its local sum-of-products definition

    local_expression_form[signal] is a list of products.
    Each product is a list of literals, for example:
      ["a", "b'"]   means   a * b'

    For a .names block:
      .names a b out
      1- 1
      -1 1

    we store:
      local_expression_form["out"] = [["a"], ["b"]]
"""
def parse_blif(blif_path):

    primary_inputs = []
    primary_outputs = []

    # Maps output signal name -> list of product terms
    # Each product term is a list of literals
    local_expression_form = defaultdict(list)

    # These track the current .names block we are inside
    current_inputs = None
    current_output = None

    with open(blif_path, "r") as blif_file:
        for raw_line in blif_file:
            line = raw_line.strip()  # remove leading and trailing whitespace

            # Skip comments, empty lines, and .model
            if line.startswith(".model") or line.startswith("#") or not line:
                continue

            if line.startswith(".inputs"):
                inputs = line.split()
                primary_inputs.extend(inputs[1:])
                continue

            if line.startswith(".outputs"):
                outputs = line.split()
                primary_outputs.extend(outputs[1:])
                continue

            if line.startswith(".names"):
                product = line.split()  # split on whitespace
                current_inputs = product[1:-1]  # all but first and last
                current_output = product[-1]  # only the last
                continue

            if line.startswith(".end"):
                break

            # Any other directive line resets the current block
            if line.startswith("."):
                current_inputs = None
                current_output = None
                continue

            # If we are not inside a .names block, ignore the line
            if current_inputs is None or current_output is None:
                continue

            # Truth table lines following .names blocks. Define as a cube and a result bit.
            truth_map = line.split()
            if len(truth_map) == 1:
                cube = ""
                result_bit = truth_map[0]
            else:
                cube, result_bit = truth_map[0], truth_map[1]

            # We only keep rows where the output is 1 because they are the ON-set product terms
            if result_bit != "1":
                continue

            # Convert the cube into one product term
            #
            # bit == "1"  -> positive literal
            # bit == "0"  -> negated literal
            # bit == "-"  -> don't care, so omit that variable
            product = []
            for bit, input_var in zip(cube, current_inputs):
                if bit == "1":
                    product.append(input_var)
                elif bit == "0":
                    product.append(input_var + "'")
                elif bit == "-":
                    pass
                else:
                    raise ValueError(f"Unexpected cube bit {bit!r}")

            # Store this product as part of the current output's local expression
            local_expression_form[current_output].append(product)

    return primary_inputs, primary_outputs, local_expression_form


"""Return the variable name without a trailing negation mark."""
def literal_base_name(literal):
    return literal[:-1] if literal.endswith("'") else literal


"""Create a literal expression node."""
def make_literal(literal):
    return ("lit", literal)


"""
    Create a sum expression node.
"""
def make_sum(terms):
    flattened_terms = []
    seen = set()

    for term in terms:
        if term[0] == "sum":
            children = term[1]
        else:
            children = [term]

        for child in children:
            key = expr_to_key(child)
            if key not in seen:
                seen.add(key)
                flattened_terms.append(child)

    if not flattened_terms:
        return ("sum", [])

    if len(flattened_terms) == 1:
        return flattened_terms[0]

    return ("sum", flattened_terms)


"""
    Create a product expression node.
"""
def make_product(factors):
    flattened_factors = []
    seen = set()

    for factor in factors:
        if factor[0] == "prod":
            children = factor[1]
        else:
            children = [factor]

        for child in children:
            key = expr_to_key(child)
            if key not in seen:
                seen.add(key)
                flattened_factors.append(child)

    if not flattened_factors:
        return ("prod", [])

    if len(flattened_factors) == 1:
        return flattened_factors[0]

    return ("prod", flattened_factors)


"""Build a stable structural key for deduplication."""
def expr_to_key(expression):
    kind = expression[0]

    if kind == "lit":
        return ("lit", expression[1])

    children = tuple(expr_to_key(child) for child in expression[1])
    return (kind, children)


"""
    Convert one local product term into a product expression.

    Example:
      ["a", "b'"]  ->  a b'
"""
def product_to_expression(product):
    factors = [make_literal(literal) for literal in product]
    return make_product(factors)


"""
    Convert one signal's local definition into a sum expression.

    Example:
      [["a"], ["b"]]  ->  (a+b)
"""
def signal_to_local_expression(signal, local_expression_form):
    if signal not in local_expression_form:
        raise ValueError(f"No definition found for signal {signal}")

    local_products = local_expression_form[signal]
    sum_terms = [product_to_expression(product) for product in local_products]
    return make_sum(sum_terms)


"""
    Recursively substitute internal nodes until the expression contains
    only primary inputs.

    This preserves structure:
      - products stay products
      - sums stay sums

    It also supports negated internal nodes by expanding the positive form
    and then applying DeMorgan's law.
"""
def expand_expression(expression, local_expression_form, primary_inputs, memo):
    kind = expression[0]

    if kind == "lit":
        literal = expression[1]
        base = literal_base_name(literal)

        # If the literal is already a primary input, keep it
        if base in primary_inputs:
            return expression

        # Expand internal node
        if base not in local_expression_form:
            raise ValueError(f"Unknown signal {base}")

        expanded_signal = expand_signal(base, local_expression_form, primary_inputs, memo)

        # If this literal is negated, negate the expanded expression
        if literal.endswith("'"):
            return negate_expression(expanded_signal)

        return expanded_signal

    if kind == "sum":
        expanded_terms = []
        for term in expression[1]:
            expanded_terms.append(expand_expression(term, local_expression_form, primary_inputs, memo))
        return make_sum(expanded_terms)

    if kind == "prod":
        expanded_factors = []
        for factor in expression[1]:
            expanded_factors.append(expand_expression(factor, local_expression_form, primary_inputs, memo))
        return make_product(expanded_factors)

    raise ValueError(f"Unknown expression kind: {kind}")


"""
    Expand one signal into a factored expression over primary inputs only.

    Memoization is used so we do not re-expand the same signal repeatedly.
"""
def expand_signal(signal, local_expression_form, primary_inputs, memo):

    # If already expanded before, reuse the cached result
    if signal in memo:
        return memo[signal]

    local_expression = signal_to_local_expression(signal, local_expression_form)
    expanded_signal = expand_expression(local_expression, local_expression_form, primary_inputs, memo)

    memo[signal] = expanded_signal
    return expanded_signal


"""
    Convert the top-level product into one factor per line.

    Output format:
      - literals in a product factor are written on one line separated by spaces
      - sums are also written on one line, using the product terms separated by spaces

    Examples:
      c            -> "c"
      d'           -> "d'"
      (a+b)        -> "a b"
      (ab'+cd)     -> "a b' | c d"

    The final example above is included only so the script has a fallback if
    a sum factor contains multi-literal product terms.
"""
def expression_to_factor_lines(expression):
    kind = expression[0]

    # If the whole expression is a single literal, print one line
    if kind == "lit":
        return [expression[1]]

    # If the whole expression is a sum, print it as one factor line
    if kind == "sum":
        return [sum_expression_to_line(expression)]

    # If the whole expression is a product, print one factor per line
    if kind == "prod":
        factor_lines = []
        for factor in expression[1]:
            if factor[0] == "lit":
                factor_lines.append(factor[1])
            elif factor[0] == "sum":
                factor_lines.append(sum_expression_to_line(factor))
            elif factor[0] == "prod":
                factor_lines.append(product_expression_to_line(factor))
            else:
                raise ValueError(f"Unknown factor kind: {factor[0]}")
        return factor_lines

    raise ValueError(f"Unknown expression kind: {kind}")


"""
    Convert a product expression into one line of literals separated by spaces.

    Example:
      ab'c   ->   "a b' c"
"""
def product_expression_to_line(expression):
    if expression[0] == "lit":
        return expression[1]

    if expression[0] != "prod":
        raise ValueError("product_expression_to_line expected a product or literal")

    literals = []
    for factor in expression[1]:
        if factor[0] == "lit":
            literals.append(factor[1])
        else:
            # Nested non-literal factors inside a product are not expected here
            literals.append(expression_to_inline_string(factor))

    return " ".join(literals)


"""
    Convert a sum expression into one line.

    Example:
      (a+b) -> "a b"

    If a sum term is itself a multi-literal product, we keep that term grouped
    using " | " between sum terms as a fallback representation.
"""
def sum_expression_to_line(expression):
    if expression[0] == "lit":
        return expression[1]

    if expression[0] != "sum":
        raise ValueError("sum_expression_to_line expected a sum or literal")

    simple_terms = []
    all_terms_are_single_literals = True

    for term in expression[1]:
        if term[0] == "lit":
            simple_terms.append(term[1])
        else:
            all_terms_are_single_literals = False
            break

    if all_terms_are_single_literals:
        return " ".join(simple_terms)

    grouped_terms = []
    for term in expression[1]:
        grouped_terms.append(expression_to_inline_string(term))

    return " | ".join(grouped_terms)


"""
    Convert an expression into a compact inline string.

    This is used as a fallback for more complex factors.
"""
def expression_to_inline_string(expression):
    kind = expression[0]

    if kind == "lit":
        return expression[1]

    if kind == "sum":
        return "(" + "+".join(expression_to_inline_string(term) for term in expression[1]) + ")"

    if kind == "prod":
        return "".join(
            expression_to_inline_string(factor) if factor[0] != "sum"
            else "(" + "+".join(expression_to_inline_string(term) for term in factor[1]) + ")"
            for factor in expression[1]
        )

    raise ValueError(f"Unknown expression kind: {kind}")

"""
    Negate an expression using DeMorgan's law.

    Examples:
      a        -> a'
      a'       -> a
      (a+b)    -> a'b'
      ab       -> (a'+b')
"""
def negate_expression(expression):
    kind = expression[0]

    if kind == "lit":
        literal = expression[1]
        if literal.endswith("'"):
            return make_literal(literal[:-1])
        return make_literal(literal + "'")

    if kind == "sum":
        negated_terms = [negate_expression(term) for term in expression[1]]
        return make_product(negated_terms)

    if kind == "prod":
        negated_factors = [negate_expression(factor) for factor in expression[1]]
        return make_sum(negated_factors)

    raise ValueError(f"Unknown expression kind: {kind}")


"""Rename all literals in an expression tree using a mapping dict."""
def rename_expression(expression, rename_map):
    kind = expression[0]

    if kind == "lit":
        literal = expression[1]
        if literal.endswith("'"):
            base = literal[:-1]
            return ("lit", rename_map.get(base, base) + "'")
        else:
            return ("lit", rename_map.get(literal, literal))

    children = [rename_expression(child, rename_map) for child in expression[1]]
    return (kind, children)


"""Command-line entry point."""
def main():

    if len(sys.argv) not in (2, 3):
        print("Usage: python3 blif_to_working_format.py input.blif [output.txt]")
        sys.exit(1)

    blif_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) == 3 else None

    # Parse BLIF into local signal definitions
    primary_inputs, primary_outputs, local_expression_form = parse_blif(blif_path)

    if not primary_outputs:
        raise ValueError("No .outputs found in BLIF")

    # For now, use the first declared primary output as the target function
    target_function = primary_outputs[0]

    # Expand target recursively into primary-input-only factored form
    memo = {}
    expanded_signal = expand_signal(target_function, local_expression_form, set(primary_inputs), memo)

    # Rename variables from original names (a, b, ...) to x1, x2, ...
    rename_map = {name: f"x{i+1}" for i, name in enumerate(primary_inputs)}
    expanded_signal = rename_expression(expanded_signal, rename_map)

    # Convert final expression into one factor per line
    lines = expression_to_factor_lines(expanded_signal)

    # Write either to a file or to stdout
    if output_path:
        with open(output_path, "w") as out_file:
            for line in lines:
                out_file.write(line + "\n")
    else:
        for line in lines:
            print(line)


if __name__ == "__main__":
    main()
