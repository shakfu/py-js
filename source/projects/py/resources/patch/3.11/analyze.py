from pprint import pprint


def analyze(path):
    d = dict(
        shared=[],
        static=[],
        disabled=[],
    )

    in_shared = False
    in_static = False
    in_disabled = False

    with open(path) as f:
        lines = f.readlines()
        lines = [line.strip() for line in lines if not line.startswith("#")]
        lines = [line for line in lines if line]  # remove empty lines
        for line in lines:
            print(line)
            if line.startswith("*shared*"):
                in_shared = True
                in_static = False
                in_disabled = False
                continue

            if line.startswith("*static*"):
                in_static = True
                in_shared = False
                in_disabled = False
                continue

            if line.startswith("*disabled*"):
                in_disabled = True
                in_static = False
                in_shared = False
                continue

            if in_shared:
                d["shared"].append(line)

            if in_static:
                d["static"].append(line)

            if in_disabled:
                d["disabled"].append(line)

        # return set(xs)
        return d


stdlib = analyze("Setup.stdlib")
pprint(stdlib)
# setup_310 = analyze('Setup.3.10')
