# This script converts a BLIF file into sum-of-products form.
#
# The output is one product term per line, using:
#   var    for positive literal
#   var'   for negated literal
#
# Example:
#   .names a b out
#   1- 1
#   -1 1
#
# becomes the local SOP:
#   a
#   b
#
# Then the script recursively expands internal nodes so the final output
# is written only in terms of primary inputs.

import sys
from collections import defaultdict

"""
    Read a BLIF file and extract:
      - primary input names
      - primary output names
      - a dictionary mapping each signal name to its local SOP definition

    definitions[signal] is a list of products.
    Each product is a list of literals, for example:
      ["a", "b'"]   means   a * b'

    For a .names block:
      .names a b out
      1- 1
      -1 1

    we store:
      definitions["out"] = [["a"], ["b"]]
"""
def parse_blif(blif_path):
    
    primary_inputs = []
    primary_outputs = []

    # Maps output signal name -> list of product terms
    # Each product term is a list of literals
    SoP_form = defaultdict(list)

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
                product = line.split() # split on whitespace
                current_inputs = product[1:-1] # all but first and last
                current_output = product[-1] # only the last
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

            # Store this product as part of the current output's SOP
            SoP_form[current_output].append(product)

    return primary_inputs, primary_outputs, SoP_form

"""
    Return the variable name without negation mark.
"""
def literal_base_name(lit):
    return lit[:-1] if lit.endswith("'") else lit

"""
    Merge two product terms into one larger product term.

    Example:
      ["a", "c"] merged with ["b'"] -> ["a", "b'", "c"]

    Unless there is a contradiction, then return None.
"""
def merge_products(prod1, prod2):
    
    assignment = {}

    # Read literals from both products and build a variable -> polarity map
    for literal in prod1 + prod2:
        if literal.endswith("'"):
            variable = literal[:-1]
            value = 0
        else:
            variable = literal
            value = 1

        # Contradiction: same variable appears both positive and negative
        if variable in assignment and assignment[variable] != value:
            return None

        assignment[variable] = value

    # Rebuild the merged product in sorted variable order
    merged_product = []
    for variable in sorted(assignment.keys()):
        merged_product.append(variable if assignment[variable] == 1 else variable + "'")

    return merged_product

"""
    Expand one product term until it contains only primary inputs.

    Example:
      if product = ["c_and_notd", "a_or_b"]
      and:
        c_and_notd -> [["c", "d'"]]
        a_or_b     -> [["a"], ["b"]]

      then expansion returns:
        [["a", "c", "d'"], ["b", "c", "d'"]]
"""
def expand_product(product, SoP_form, primary_inputs, memo):
    
    # Start with one empty partial product
    expanded_products = [[]]

    # Process each literal in the product one at a time
    for literal in product:
        base = literal_base_name(literal)

        # If this literal is already a primary input, just merge it in
        if base in primary_inputs:
            new_expanded = []
            for product in expanded_products:
                merged_product = merge_products(product, [literal])
                if merged_product is not None:
                    new_expanded.append(merged_product)
            expanded_products = new_expanded
            continue

        # Negated internal nodes are not supported in this simple version
        if literal.endswith("'"):
            raise ValueError(f"Negated internal node not supported: {literal}")

        # Internal signal must have a definition
        if base not in SoP_form:
            raise ValueError(f"Unknown signal {base}")

        # Recursively expand that internal signal
        subproducts = expand_signal(base, SoP_form, primary_inputs, memo)

        # Distribute:
        # current partial products  X  subproducts of this signal
        new_expanded = []
        for product in expanded_products:
            for subproduct in subproducts:
                merged_product = merge_products(product, subproduct)
                if merged_product is not None:
                    new_expanded.append(merged_product)

        expanded_products = new_expanded

    return expanded_products

"""
    Expand one signal into a list of product terms over primary inputs only.

    Memoization is used so we do not re-expand the same signal repeatedly.
"""
def expand_signal(signal, SoP_form, primary_inputs, memo):
    
    # If already expanded before, reuse the cached result
    if signal in memo:
        return memo[signal]

    if signal not in SoP_form:
        raise ValueError(f"No definition found for signal {signal}")

    expanded_signal = []

    # Expand each local product term in the signal's definition
    for product in SoP_form[signal]:
        expanded_signal.extend(expand_product(product, SoP_form, primary_inputs, memo))

    # Remove duplicate products
    unique_expansion = []
    seen = set()
    for product in expanded_signal:
        key = tuple(product)
        if key not in seen:
            seen.add(key)
            unique_expansion.append(product)

    memo[signal] = unique_expansion
    return unique_expansion

"""
    Command-line entry point.

    Usage:
      python3 blif_to_SoP.py input.blif
      python3 blif_to_SoP.py input.blif output.txt
"""
def main():
    
    if len(sys.argv) not in (2, 3):
        print("Usage: python3 blif_to_SoP.py input.blif [output.txt]")
        sys.exit(1)

    blif_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) == 3 else None

    # Parse BLIF into local signal definitions
    primary_inputs, primary_outputs, SoP_form = parse_blif(blif_path)

    if not primary_outputs:
        raise ValueError("No .outputs found in BLIF")

    # For now, use the first declared primary output as the target function
    target_function = primary_outputs[0]

    # Expand target recursively into primary-input-only SOP
    memo = {}
    expanded_signal = expand_signal(target_function, SoP_form, set(primary_inputs), memo)

    # Convert each product term to one output line
    lines = [" ".join(prod) for prod in expanded_signal]

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