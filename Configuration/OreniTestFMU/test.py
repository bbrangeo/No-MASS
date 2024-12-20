import os as O
import numpy as N
from pyfmi import load_fmu


curr_dir = O.path.dirname(O.path.abspath(__file__))
path_to_fmus = O.path.join(curr_dir, "files", "FMUs")
path_to_fmus_me1 = O.path.join(path_to_fmus, "ME1.0")
path_to_fmus_cs1 = O.path.join(path_to_fmus, "CS1.0")


def run_demo(with_plots=True):
    """
    Demonstrates how to simulate an FMU with inputs.

    See also simulation_with_input.py
    """
    fmu_name = "Simulate.fmu"

    # Load the dynamic library and XML data
    model = load_fmu(fmu_name)

    # Set the first input value to the model
    model.set("Input1", 1.0)
    input_object = ("u", 1)

    # Simulate
    res = model.simulate(final_time=30, input=input_object, options={"ncp": 3000})
    print(res)


if __name__ == "__main__":
    run_demo()
