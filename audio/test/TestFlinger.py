#!/usr/bin/env python
###
### Copyright (C) 2011 Texas Instruments
###
### Licensed under the Apache License, Version 2.0 (the "License");
### you may not use this file except in compliance with the License.
### You may obtain a copy of the License at
###
###      http://www.apache.org/licenses/LICENSE-2.0
###
### Unless required by applicable law or agreed to in writing, software
### distributed under the License is distributed on an "AS IS" BASIS,
### WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
### See the License for the specific language governing permissions and
### limitations under the License.
###

"""TestFlinger meta-test execution framework

When writing a master test script that runs several scripts, this module
can be used to execute those tests in a detached process (sandbox).
Thus, if the test case fails by a segfault or timeout, this can be
detected and the upper-level script simply moves on to the next script.
"""

import os
import time
import subprocess
import sys
import time

g_default_timeout = 300

class TestCase:
    """Test running wrapper object."""

    def __init__(self, TestDict = {}, Logfile = None):
        """Set up the test runner object.

        TestDict: dictionary with the test properties.  (string: value).  The
                  recognized properties are:

                  filename - name of executable test file
                      Type: string
                      Required: yes

                  args - command line arguments for test
                      Type: list of strings, or None
                      Required: no
                      Default: None

                  timeout - upper limit on execution time (secs).  If test takes
                      this long to run, then it is deemed a failure
                      Type: integer
                      Required: no
                      Default: TestFlinger.g_default_timeout (typ. 300 sec)

                  expect-fail - If the test is expected to fail (return non-zero)
                      in order to pass, set this to True
                      Type: bool
                      Required: no
                      Default: False

                  expect-signal If the test is expected to fail because of
                      a signal (e.g. SIGTERM, SIGSEGV) then this is considered success
                      Type: bool
                      Required: no
                      Default: False

        Logfile: a file object where stdout/stderr for the tests should be dumped.
            If null, then no logging will be done.  (See also TestFlinger.setup_logfile()
            and TestFlinger.close_logfile().
        """
        global g_default_timeout

        self._program = None
        self._args = None
        self._timeout = g_default_timeout # Default timeout
        self._verdict = None
        self._expect_fail = False
        self._expect_signal = False
        self._logfile = Logfile

        self._proc = None
        self._time_expire = None

        self._program = TestDict['filename']
        if 'args' in TestDict:
            self._args = TestDict['args']
        if 'timeout' in TestDict and TestDict['timeout'] is not None:
            self._timeout = TestDict['timeout']
        if 'expect-fail' in TestDict and TestDict['expect-fail'] is not None:
            self._expect_fail = TestDict['expect-fail']
        if 'expect-signal' in TestDict and TestDict['expect-signal'] is not None:
            self._expect_signal = TestDict['expect-signal']

    def __del__(self):
        pass

    def start(self):
        """Starts the test in another process.  Returns True if the
        test was successfully spawned.  False if there was an error.
        """

        command = os.path.abspath(self._program)

        if not os.path.exists(command):
            print "ERROR: The program to execute does not exist (%s)" % (command,)
            return False

        timestamp = time.strftime("%Y.%m.%d %H:%M:%S")
        now = time.time()
        self._time_expire = self._timeout + now
        self._kill_timeout = False

        self._log_write("====================================================================\n")
        self._log_write("BEGINNG TEST '%s' at %s\n" % (self._program, timestamp))
        self._log_write("--------------------------------------------------------------------\n")
        self._log_flush()

        self._proc = subprocess.Popen(args=command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

        return (self._proc is not None)

    def wait(self):
        """Blocks until the test completes or times out, whichever
        comes first.  If test fails, returns False.  Otherwise returns
        true.
        """

        if self._proc is None:
            print "ERROR: Test was never started"
            return False

        self._proc.poll()
        while (time.time() < self._time_expire) and (self._proc.poll() is None):
            self._process_logs()
            time.sleep(.5)

        if self._proc.returncode is None:
            self.kill()
            return False

        self._process_logs()
        self._finalize_log()

        return True

    def kill(self):
        """Kill the currently running test (if there is one).
        """

        if self._proc is None:
            print "WARNING: killing a test was never started"
            return False

        self._kill_timeout = True
        self._proc.terminate()
        time.sleep(2)
        self._proc.kill()
        self._log_write("\nKilling process by request...\n")
        self._log_flush()
        self._finalize_log()

        return True


    def verdict(self):
        """Returns a string, either 'PASS', 'FAIL', 'FAIL/TIMEOUT', or 'FAIL/SIGNAL(n)
        '"""
        self._proc.poll()

        rc = self._proc.returncode

        if rc is None:
            print "ERROR: test is still running"

        if self._kill_timeout:
            return "FAIL/TIMOUT"

        if rc < 0 and self._expect_signal:
            return "PASS"
        elif rc < 0:
            return "FAIL/SIGNAL(%d)" % (-rc,)

        if self._expect_fail:
            if rc != 0:
                return "PASS"
            else:
                return "FAIL"
        else:
            if rc == 0:
                return "PASS"
            else:
                return "FAIL"

    def _process_logs(self):
        if self._logfile is not None:
            data = self._proc.stdout.read()
            self._logfile.write(data)
            self._logfile.flush()

    def _finalize_log(self):
        timestamp = time.strftime("%Y.%m.%d %H:%M:%S")
        self._log_write("--------------------------------------------------------------------\n")
        self._log_write("ENDING TEST '%s' at %s\n" % (self._program, timestamp))
        self._log_write("====================================================================\n")
        self._log_flush()

    def _log_write(self, data):
        if self._logfile is not None:
            self._logfile.write(data)

    def _log_flush(self):
        if self._logfile is not None:
            self._logfile.flush()

def setup_logfile(override_logfile_name = None):
    """Open a logfile and prepare it for use with TestFlinger logging.
    The filename will be generated based on the current date/time.

    If override_logfile_name is not None, then that filename will be
    used instead.

    See also: close_logfile()
    """

    tmpfile = None
    if override_logfile_name is not None:
        tmpfile = override_logfile_name
        if os.path.exists(tmpfile):
            os.unlink(tmpfile)
    else:
        tmpfile = time.strftime("test-log-%Y.%m.%d.%H%M%S.txt")
        while os.path.exists(tmpfile):
            tmpfile = time.strftime("test-log-%Y.%m.%d.%H%M%S.txt")
    fobj = open(tmpfile, 'wt')
    print "Logging to", tmpfile
    timestamp = time.strftime("%Y.%m.%d %H:%M:%S")
    fobj.write("BEGINNING TEST SET %s\n" % (timestamp,))
    fobj.write("====================================================================\n")
    return fobj

def close_logfile(fobj):
    """Convenience function for closing a TestFlinger log file.

    fobj: an open and writeable file object

    See also : setup_logfile()
    """

    timestamp = time.strftime("%Y.%m.%d %H:%M:%S")
    fobj.write("====================================================================\n")
    fobj.write("CLOSING TEST SET %s\n" % (timestamp,))

if __name__ == "__main__":
    pass
