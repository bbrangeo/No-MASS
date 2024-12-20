from pythonfmu import Fmi2Causality, Fmi2Slave, Real


class Simulate(Fmi2Slave):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.author = "Boris"
        self.guid = "{fd719ef5-c46e-48c7-ae95-96089a69ee64}"
        self.description = "A simple description"
        self.modelName = "Simulate"
        self.Input1 = 1.0
        self.version = "1.2"
        self.Output1 = 3.0
        self.register_variable(Real("Input1", causality=Fmi2Causality.input)),
        self.register_variable(
            Real("Output1", causality=Fmi2Causality.output, getter=lambda: self.Output1)
        )

    def do_step(self, current_time, step_size):
        print(self.Input1)
        print(self.Output1)
        return True
