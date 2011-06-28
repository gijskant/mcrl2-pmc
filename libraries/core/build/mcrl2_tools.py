#~ Copyright 2011 Wieger Wesselink.
#~ Distributed under the Boost Software License, Version 1.0.
#~ (See accompanying file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

import os
import random
from optparse import OptionParser
from path import *

class mcrl2_tool_options:
    tooldir = ''                # optional location of mCRL2 tools
    files_to_be_removed = {}    # files that need to be removed after testing
    verbose = False

def parse_command_line():
    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option("-t", "--tooldir", dest="tooldir", help="the mCRL2 tools directory")
    parser.add_option("-i", "--iterations", type="int", dest="iterations", default="100", help="the number of tests that is performed")
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", help="print verbose output")
    parser.add_option("-k", "--keep-files", action="store_true", dest="keep_files", help="keep temporary files")
    (options, args) = parser.parse_args()
    set_mcrl2_tooldir(options.tooldir)
    mcrl2_tool_options.verbose = options.verbose
    return options

def add_temporary_files(filename1, filename2 = None):
    mcrl2_tool_options.files_to_be_removed[filename1] = 1
    if filename2 != None:
        mcrl2_tool_options.files_to_be_removed[filename2] = 1

def remove_temporary_files():
    for filename in mcrl2_tool_options.files_to_be_removed.keys():
        if path(filename).exists():
            if mcrl2_tool_options.verbose:
                print 'removing', filename
            path(filename).remove()
    mcrl2_tool_options.files_to_be_removed.clear()

def set_mcrl2_tooldir(tooldir):
    if tooldir != None:
        mcrl2_tool_options.tooldir = tooldir

def add_tool_dir(toolname):
    if mcrl2_tool_options.tooldir != '':
        return path(mcrl2_tool_options.tooldir) / toolname / toolname
    return toolname

def run_program(program, options, redirect = None):
    program = add_tool_dir(program)
    arg = '%s %s' % (program, options)
    if redirect != None:
        if os.name == 'nt':
            arg = arg + ' 2> %s 1>&2' % redirect
        else:
            arg = arg + ' >& %s' % redirect
    if mcrl2_tool_options.verbose:
        print 'executing', arg
    os.system(arg)

def timeout_command(program, options, timeout = -1):
    """call shell-command and either return its output or kill it
    if it doesn't normally exit within timeout seconds and return None"""
    import subprocess, datetime, os, time, signal, platform

    program = add_tool_dir(program)
    command = program + ' ' + options

    if mcrl2_tool_options.verbose:
        print 'executing', command

    cmd = command.split(" ")
    start = datetime.datetime.now()
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    while process.poll() is None:
        time.sleep(0.1)
        now = datetime.datetime.now()
        if (now - start).seconds > timeout:
            if platform.system() == 'Windows':
                #os.kill(process.pid, signal.CTRL_C_EVENT)
                #os.kill(process.pid, signal.CTRL_BREAK_EVENT)
                os.kill(process.pid, signal.SIGABRT)
            else:
                os.kill(process.pid, signal.SIGKILL)
                os.waitpid(-1, os.WNOHANG)
            return None, None

    return (process.stdout.read(), process.stderr.read())

def last_word(line):
    words = line.strip().split()
    return words[len(words) - 1]

# returns True, False or None if a timeout occurs
def run_pbes2bool(filename, timeout = 3):
    add_temporary_files(filename)
    dummy, text = timeout_command('pbes2bool', filename, timeout)
    if text == None:
        print 'WARNING: timeout on %s' % filename
        return None
    return last_word(text) == 'true'

# returns True, False or None if a timeout occurs
def run_pbespgsolve(filename, timeout = 3):
    add_temporary_files(filename)
    dummy, text = timeout_command('pbespgsolve', filename, timeout)
    if text == None:
        print 'WARNING: timeout on %s' % filename
        return None
    return last_word(text) == 'true'

def run_txt2pbes(txtfile, pbesfile):
    add_temporary_files(txtfile, pbesfile)
    run_program('txt2pbes', '%s %s' % (txtfile, pbesfile))

def run_pbesabstract(pbesfile1, pbesfile2, abstraction_value, selection = '*(*:*)'):
    add_temporary_files(pbesfile1, pbesfile2)
    run_program('pbesabstract', '--select="%s" --abstraction-value=%d %s %s' % (selection, abstraction_value, pbesfile1, pbesfile2))

def run_pbesconstelm(pbesfile1, pbesfile2, rewriter = 'quantifier-all', timeout = 10):
    add_temporary_files(pbesfile1, pbesfile2)
    timeout_command('pbesconstelm',  '-p%s %s %s' % (rewriter, pbesfile1, pbesfile2), timeout)

def run_pbesparelm(pbesfile1, pbesfile2, timeout = 10):
    add_temporary_files(pbesfile1, pbesfile2)
    timeout_command('pbesparelm',  '%s %s' % (pbesfile1, pbesfile2), timeout)

def run_pbespareqelm(pbesfile1, pbesfile2, timeout = 10):
    add_temporary_files(pbesfile1, pbesfile2)
    timeout_command('pbespareqelm', '%s %s' % (pbesfile1, pbesfile2), timeout)

# returns the output of pbespp
def run_pbespp(pbesfile, timeout = 10):
    add_temporary_files(pbesfile)
    text, dummy = timeout_command('pbespp', pbesfile, timeout)
    return text

# returns True if the operation succeeded within the given amount of time
def run_pbesinst(pbesfile, besfile, strategy = 'lazy', selection = '', timeout = 10):
    add_temporary_files(pbesfile, besfile)
    options = '-s%s' % (strategy)
    if selection != '':
        options = options + ' -f"%s"' % selection
    dummy, text = timeout_command('pbesinst',  '%s %s %s' % (options, pbesfile, besfile), timeout)
    if text == None:
        print 'WARNING: timeout on %s' % pbesfile     
        return False
    if text.startswith('error'):
        print 'WARNING: pbesinst failed on %s (%s)' % (pbesfile, text)
        return False
    return True

# returns True, False or None if a timeout occurs
def run_bessolve(filename, strategy = 'spm', timeout = 10):
    add_temporary_files(filename)
    command = 'bessolve ' + '-s%s %s' % (strategy, filename)
    text, dummy = timeout_command('bessolve', '-s%s %s' % (strategy, filename), timeout)
    if text == None:
        print 'WARNING: timeout on "%s"' % command
        return None
    result = last_word(text)
    if result == 'true':
        return True
    elif result == 'false':
        return False
    print 'WARNING: unknown failure on "%s"' % command
    return None
