import sys

sys.path.append("../cmake-build-release/")

import pyuniengine as ue

def __main__():
    ue.RunApplication()
    app = Application()
    app.Create()