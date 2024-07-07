
from math import sqrt

import numpy
import miniball

class BoundingCircle:
    def __init__(self):
        self.pos_       = list()
        self.r2_        = None
        self.r_         = None
        self.px_        = None
        self.py_        = None
        self.calc_      = False

    def add_pos(self, x, y):
        self.pos_.append((x, y, 0))
        self.calc_ = False

    def calc(self):
        assert(len(self.pos_) > 0)
        if self.calc_:
            return
        np_array        = self.pos_
        np_unique_array = numpy.unique(np_array, axis=0)
        p, r2 = miniball.get_bounding_ball(np_unique_array)
        self.r_     = sqrt(r2)
        self.r2_    = r2
        self.px_    = p[0]
        self.py_    = p[1]
        self.calc_  = True

    def px(self):
        self.calc()
        return self.px_

    def py(self):
        self.calc()
        return self.py_

    def radius(self):
        self.calc()
        return self.r_

    def radius_squared(self):
        self.calc()
        return self.r2_
