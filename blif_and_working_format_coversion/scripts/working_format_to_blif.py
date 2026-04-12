# This script converts a working-format file into a BLIF file.
#
# The working format is interpreted as:
#   - one factor per line
#   - one literal on a line means a literal factor
#   - multiple literals on a line mean a sum factor
#   - "|" on a line means a sum of product terms
#
# Example:
#   c
#   d'
#   a b
#
# is interpreted as:
#   c * d' * (a+b)
#
# and converted into a BLIF network.
#
# Important note:
#   The current blif_to_working_format.py script is not fully invertible,
#   because it uses the same text format for some products and some sums.
#   This reverse script assumes that:
#       "a b" means (a+b)
#   unless "|" is present, in which case each side of "|" is treated as
#   one product term.
#
# This script also supports an optional reference BLIF file so that the
# rebuilt BLIF can preserve the original .inputs and .outputs interface.
# This is useful for ABC CEC, which expects matching primary inputs.

import sys
import os


"""
    Return the variable name without negation mark.

    Examples:
      "a"  -> "a"
      "b'" -> "b"
"""
def literal_base_name(literal):
    return literal[:-1] if literal.endswith("'") else literal


"""
    Read a working-format file and extract a list of factors.

    Each factor is stored as a list of product terms.
    Each product term is a list of literals.

    Examples:
      "d'"           -> [[ "d'" ]]
      "a b"          -> [[ "a" ], [ "b" ]]
      "a b' | c d"   -> [[ "a", "b'" ], [ "c", "d" ]]

    So:
      - a single literal line becomes one product term
      - a multi-literal line without "|" becomes a sum of single literals
      - a line with "|" becomes an explicit sum of products
"""
def parse_working_format(working_format_path):
    factors = []

    with open(working_format_path, "r") as working_file:
        for raw_line in working_file:
            line = raw_line.strip()

            # Skip comments and blank lines
            if not line or line.startswith("#"):
                continue

            # Explicit sum-of-products line
            if "|" in line:
                sum_terms = []
                for part in line.split("|"):
                    term_text = part.strip()
                    if not term_text:
                        continue
                    literals = term_text.split()
                    sum_terms.append(literals)

                if not sum_terms:
                    raise ValueError(f"Invalid working-format line: {line}")

                factors.append(sum_terms)
                continue

            # No "|" means either one literal or a sum of single literals
            literals = line.split()

            if len(literals) == 1:
                # Literal factor
                factors.append([[literals[0]]])
            else:
                # Interpret as a sum factor:
                #   "a b c" -> (a+b+c)
                factors.append([[literal] for literal in literals])

    return factors


"""
    Read only the primary inputs and outputs from a BLIF file.

    This is used when we want to preserve the original interface from
    a reference BLIF file.
"""
def parse_blif_interface(blif_path):
    primary_inputs = []
    primary_outputs = []

    with open(blif_path, "r") as blif_file:
        for raw_line in blif_file:
            line = raw_line.strip()

            if not line or line.startswith("#"):
                continue

            if line.startswith(".inputs"):
                parts = line.split()
                primary_inputs.extend(parts[1:])
                continue

            if line.startswith(".outputs"):
                parts = line.split()
                primary_outputs.extend(parts[1:])
                continue

            if line.startswith(".end"):
                break

    return primary_inputs, primary_outputs


"""
    Collect the primary input names from all literals in all factors.
"""
def collect_primary_inputs(factors):
    primary_inputs = set()

    for factor in factors:
        for product_term in factor:
            for literal in product_term:
                primary_inputs.add(literal_base_name(literal))

    return sorted(primary_inputs)


"""
    Build a BLIF writer object that accumulates .names blocks.
"""
class BlifBuilder:
    def __init__(self, model_name, primary_inputs, output_name):
        self.model_name = model_name
        self.primary_inputs = primary_inputs
        self.output_name = output_name

        self.lines = []
        self.generated_names = set()
        self.temp_index = 0

    """
        Return a unique temporary signal name.
    """
    def new_signal(self, prefix):
        self.temp_index += 1
        return f"{prefix}_{self.temp_index}"

    """
        Ensure a negated literal has a signal in the network.

        Example:
          d'  -> create:
            .names d not_d
            0 1

        and return "not_d"
    """
    def literal_to_signal(self, literal):
        if not literal.endswith("'"):
            return literal

        base = literal[:-1]
        neg_signal = f"not_{base}"

        if neg_signal not in self.generated_names:
            self.generated_names.add(neg_signal)
            self.lines.append(f".names {base} {neg_signal}")
            self.lines.append("0 1")
            self.lines.append("")

        return neg_signal

    """
        Create a BLIF node for one product term.

        A product term is an AND of literals.

        Examples:
          ["a"]        -> just return signal a
          ["d'"]       -> just return signal not_d
          ["a", "d'"]  -> create an AND node
    """
    def build_product_term(self, product_term, prefix):
        input_signals = [self.literal_to_signal(literal) for literal in product_term]

        if len(input_signals) == 0:
            # Constant 1 term
            term_signal = self.new_signal(prefix)
            self.lines.append(f".names {term_signal}")
            self.lines.append("1")
            self.lines.append("")
            return term_signal

        if len(input_signals) == 1:
            return input_signals[0]

        term_signal = self.new_signal(prefix)
        self.generated_names.add(term_signal)

        self.lines.append(f".names {' '.join(input_signals)} {term_signal}")
        self.lines.append(f"{'1' * len(input_signals)} 1")
        self.lines.append("")

        return term_signal

    """
        Create a BLIF node for one factor.

        A factor is a sum of product terms.

        Examples:
          [[ "d'" ]]                 -> d'
          [[ "a" ], [ "b" ]]         -> (a+b)
          [[ "a", "b'" ], [ "c" ]]   -> ab' + c
    """
    def build_factor(self, factor, factor_index):
        term_signals = []

        for term_index, product_term in enumerate(factor, start=1):
            term_signal = self.build_product_term(product_term, f"factor{factor_index}_term{term_index}")
            term_signals.append(term_signal)

        if len(term_signals) == 1:
            return term_signals[0]

        factor_signal = self.new_signal(f"factor{factor_index}_or")
        self.generated_names.add(factor_signal)

        self.lines.append(f".names {' '.join(term_signals)} {factor_signal}")

        # OR of the term signals:
        # one row per term with that term equal to 1 and the others don't-care
        for idx in range(len(term_signals)):
            cube = ["-"] * len(term_signals)
            cube[idx] = "1"
            self.lines.append(f"{''.join(cube)} 1")

        self.lines.append("")
        return factor_signal

    """
        Build the final output as an AND of all factor signals.
    """
    def build_output(self, factor_signals):
        if len(factor_signals) == 0:
            self.lines.append(f".names {self.output_name}")
            self.lines.append("0")
            self.lines.append("")
            return

        if len(factor_signals) == 1:
            self.lines.append(f".names {factor_signals[0]} {self.output_name}")
            self.lines.append("1 1")
            self.lines.append("")
            return

        self.lines.append(f".names {' '.join(factor_signals)} {self.output_name}")
        self.lines.append(f"{'1' * len(factor_signals)} 1")
        self.lines.append("")

    """
        Return the full BLIF text.
    """
    def build_text(self):
        header = [
            f".model {self.model_name}",
            f".inputs {' '.join(self.primary_inputs)}",
            f".outputs {self.output_name}",
            ""
        ]

        footer = [".end"]

        return "\n".join(header + self.lines + footer)


"""
    Convert a working-format file into a BLIF file.

    If a reference BLIF path is provided, the rebuilt BLIF uses the
    original .inputs and .outputs from that file. This helps ABC CEC
    compare the two circuits even when some inputs are redundant.
"""
def working_format_to_blif(
    working_format_path,
    blif_path,
    output_name="f",
    model_name=None,
    reference_blif_path=None
):
    factors = parse_working_format(working_format_path)

    if reference_blif_path is not None:
        primary_inputs, reference_outputs = parse_blif_interface(reference_blif_path)

        if len(reference_outputs) > 0:
            output_name = reference_outputs[0]
    else:
        primary_inputs = collect_primary_inputs(factors)

    if model_name is None:
        base_name = os.path.basename(blif_path)
        model_name = os.path.splitext(base_name)[0]

    builder = BlifBuilder(model_name, primary_inputs, output_name)

    factor_signals = []
    for factor_index, factor in enumerate(factors, start=1):
        factor_signal = builder.build_factor(factor, factor_index)
        factor_signals.append(factor_signal)

    builder.build_output(factor_signals)

    with open(blif_path, "w") as blif_file:
        blif_file.write(builder.build_text() + "\n")


"""
    Command-line entry point.

    Usage:
      python working_format_to_blif.py input.txt output.blif
      python working_format_to_blif.py input.txt output.blif output_name
      python working_format_to_blif.py input.txt output.blif output_name model_name
      python working_format_to_blif.py input.txt output.blif output_name model_name reference.blif
"""
def main():
    if len(sys.argv) not in (3, 4, 5, 6):
        print("Usage:")
        print("  python working_format_to_blif.py input.txt output.blif")
        print("  python working_format_to_blif.py input.txt output.blif output_name")
        print("  python working_format_to_blif.py input.txt output.blif output_name model_name")
        print("  python working_format_to_blif.py input.txt output.blif output_name model_name reference.blif")
        sys.exit(1)

    working_format_path = sys.argv[1]
    blif_path = sys.argv[2]
    output_name = sys.argv[3] if len(sys.argv) >= 4 else "f"
    model_name = sys.argv[4] if len(sys.argv) >= 5 else None
    reference_blif_path = sys.argv[5] if len(sys.argv) >= 6 else None

    working_format_to_blif(
        working_format_path,
        blif_path,
        output_name,
        model_name,
        reference_blif_path
    )


if __name__ == "__main__":
    main()
