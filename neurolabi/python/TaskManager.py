import uuid
import os
import sys

class Scheduler:
    def __init__(self):
        home = os.path.expanduser('~')
        self.jobDir = os.path.join(home, 'local/scheduled')

    def submit(self, command, dependency = None, finishedFlag = None):
        if not isinstance(dependency, list):
            print 'depencency must be a list. Abort'
            return

        jobPath = os.path.join(self.jobDir, str(uuid.uuid4()) + '.sh')
        f = open(jobPath, 'w')
        if f:
            if dependency:
                for dep in dependency:
                    f.write('if [ -f "' + dep + '" ]; then\n')

            f.write('touch ' + jobPath + '.started\n')
            if isinstance(command, str):
                f.write(command + '\n')
            elif isinstance(command, list):
                for line in command:
                    f.write(line + '\n')

            if finishedFlag:
                f.write('if [ -f "' + finishedFlag + '" ]; then\n')
                
            f.write('touch ' + jobPath + '.finished\n')

            if finishedFlag:
                f.write('fi\n')

            if dependency:
                for dep in dependency:
                    f.write('fi\n')

            f.close()


if __name__ == '__main__':
    sd = Scheduler()
    sd.submit('ls')
    sd.submit(['ls', 'ps'], dependency = '/tmp/test.dep')
