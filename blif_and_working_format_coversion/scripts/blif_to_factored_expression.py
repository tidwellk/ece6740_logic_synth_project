# This script converts a BLIF file into a factored Boolean expression.
#
# The goal is to preserve products and sums instead of expanding everything
# into a flat sum-of-products form.
#
# Example:
#   .names a b out
#   1- 1
#   -1 1
#
# becomes the local expression:
#   (a+b)
#
# Then the script recursively expands internal nodes so the final output
# is written only in terms of primary inputs.
#
# Usage:
#   python3 blif_to_factored_expression.py input.blif
#   python3 blif_to_factored_expression.py input.blif output.txt

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


"""Create a sum expression node with flattened duplicate-free children."""
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


"""Create a product expression node with flattened duplicate-free children."""
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
def expr_to_key(expr):
    kind = expr[0]

    if kind == "lit":
        return ("lit", expr[1])

    children = tuple(expr_to_key(child) for child in expr[1])
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

    It does not distribute sums over products.
"""
def expand_expression(expression, local_expression_form, primary_inputs, memo):
    kind = expression[0]

    if kind == "lit":
        literal = expression[1]
        base = literal_base_name(literal)

        # If the literal is already a primary input, keep it
        if base in primary_inputs:
            return expression

        # Negated internal nodes are not supported in this simple version
        if literal.endswith("'"):
            raise ValueError(f"Negated internal node not supported: {literal}")

        # Otherwise recursively expand the internal signal
        return expand_signal(base, local_expression_form, primary_inputs, memo)

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
    Convert the expression tree into a readable Boolean expression string.

    Style:
      - literal: a or b'
      - product: ab'c
      - sum:     (a+b+c)
"""
def expression_to_string(expression):
    kind = expression[0]

    if kind == "lit":
        return expression[1]

    if kind == "sum":
        children = expression[1]
        return "(" + "+".join(expression_to_string(child) for child in children) + ")"

    if kind == "prod":
        children = expression[1]
        parts = []

        for child in children:
            child_string = expression_to_string(child)

            # Put parentheses around sums when they appear inside products
            if child[0] == "sum":
                parts.append(child_string)
            else:
                parts.append(child_string)

        return "".join(parts)

    raise ValueError(f"Unknown expression kind: {kind}")


"""Command-line entry point."""
def main():

    if len(sys.argv) not in (2, 3):
        print("Usage: python3 blif_to_factored_expression.py input.blif [output.txt]")
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

    # Convert final expression to a printable string
    final_expression = expression_to_string(expanded_signal)

    # Write either to a file or to stdout
    if output_path:
        with open(output_path, "w") as out_file:
            out_file.write(final_expression + "\n")
    else:
        print(final_expression)


if __name__ == "__main__":
    main()
    
