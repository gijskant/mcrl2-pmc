#~ Copyright 2010, 2011 Wieger Wesselink.
#~ Distributed under the Boost Software License, Version 1.0.
#~ (See accompanying file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

from path import *
from random_pbes_generator import *
from mcrl2_tools import *

def test_pbesinst_finite(p, filename):
    txtfile = filename + '.txt'
    path(txtfile).write_text('%s' % p)
    pbesfile1 = filename + '.pbes'
    pbesfile2 = filename + 'a.pbes'
    run_txt2pbes(txtfile, pbesfile1)
    if run_pbesinst(pbesfile1, pbesfile2, strategy = 'finite', selection = '*(*:Bool)'):
        answer1 = run_pbes2bool(pbesfile1)
        answer2 = run_pbes2bool(pbesfile2)
        print filename, answer1, answer2   
        if answer1 == None or answer2 == None:
          return True
        return answer1 == answer2
    return True

def main():
    options = parse_command_line()
    try:
        equation_count = 2
        atom_count = 2
        propvar_count = 2
        use_quantifiers = True
        
        for i in range(options.iterations):
            filename = 'pbesinst_finite_%d' % i
            p = make_pbes(equation_count, atom_count, propvar_count, use_quantifiers)
            if not test_pbesinst_finite(p, filename):
                m = CounterExampleMinimizer(p, lambda x: test_pbesinst(x, filename + '_minimize'), 'pbesinst_finite')
                m.minimize()
                print 'Test %s.txt failed' % filename
                exit(1)
    finally:
        if not options.keep_files:
            remove_temporary_files()

if __name__ == '__main__':
    main()
