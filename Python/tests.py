import unittest
from AST import *
import lexer

'''
Todo:
    - test all the __eq__ methods of the AST Nodes

    - test the existing parser to use as regression tests as other features are added

    - add tests for the lists, arrays, strings, etc. as you add them

    - may want to add lexer tests for both the python and cpp implementation


'''

class TestAstEquality(unittest.TestCase):
    def test_literals_eq(self):

        sl1 = Literal( 'string-literal', 'hello' )
        sl2 = Literal( 'string-literal', 'goodbye' )
        sl3 = Literal( 'string-literal', 'hello' )
        il1 = Literal( 'int-literal', 5 )
        il2 = Literal( 'int-literal', 5 )
        il3 = Literal( 'int-literal', 77 )

        self.assertEqual(sl1, sl3)
        self.assertEqual(il1, il2)
        self.assertNotEqual(sl1, sl2)
        self.assertNotEqual(sl3, il3)
        self.assertNotEqual(sl1, sl2)
        self.assertNotEqual(il2, il3)
        
    def test_var_lookup(self):

        v1 = VariableLookup( 'variableName1' )
        v2 = VariableLookup( 'variableName2' )
        v3 = VariableLookup( 'variableName1' )

        self.assertEqual(v1, v3)
        self.assertNotEqual(v1, v2)
        self.assertNotEqual(v3, Literal('bool-literal', 'false'))

    def test_ops(self):

        bo1 = BinaryOp('+', VariableLookup('a'), VariableLookup('b'))
        bo2 = BinaryOp('+', VariableLookup('a'), VariableLookup('b'))
        bo3 = BinaryOp('-', VariableLookup('a'), VariableLookup('b'))
        bo4 = BinaryOp('*', bo1, bo3)
        bo5 = BinaryOp('*', bo2, bo3)
        uo1 = UnaryOp('-', bo1)
        uo2 = UnaryOp('-', bo2)
        uo3 = UnaryOp('-', bo3)

        self.assertEqual(bo1, bo2)
        self.assertEqual(bo4, bo5)
        self.assertEqual(uo1, uo2)
        self.assertNotEqual(uo1, uo3)
        self.assertNotEqual(bo2, bo3)
        self.assertNotEqual(bo3, bo4)

    def test_fn_call(self):

        FC = FunctionCall 

        num1 = Literal('real-literal', '10.0')
        num2 = Literal('real-literal', '10.0')
        num3 = Literal('real-literal', '77.677')
        var1 = VariableLookup('fyodor')
        var2 = VariableLookup('fyodor')
        var3 = VariableLookup('mitya')
        bo1 = BinaryOp('/', num3, num1)
        bo2 = BinaryOp('/', num3, num1)
        bo3 = BinaryOp('*', num3, num1)

        self.assertEqual( FC('fn_name', [num1, num3]), FC('fn_name', [num1, num3]) )
        self.assertEqual( FC('fn_name', [num1, num3]), FC('fn_name', [num2, num3]) )
        self.assertNotEqual( FC('fn_name', [num1, num3]), FC('blah', [num1, num3]) )
        self.assertNotEqual( FC('fn_name', [num1, num3]), FC('fn_name', [num1]) )
        self.assertNotEqual( FC('fn_name', [num1, num3]), FC('fn_name', [num1, var1]) )
        self.assertEqual( FC('fn_name', [bo1]), FC('fn_name', [bo2]) )
        self.assertNotEqual( FC('fn_name', [bo1]), FC('fn_name', [bo3]) )
        self.assertEqual( FC('fn_name', [num1, var1, bo1]), FC('fn_name', [num2, var2, bo2]) )
        self.assertNotEqual( FC('fn_name', [num1, var1, bo1]), FC('fn_name', [bo2, num2, var2]) )
        self.assertEqual( FC('fn', [ FC('fn', [num1]) ]), FC('fn', [ FC('fn', [num2]) ]) )

    def test_assign_declare(self):
        ''' 
        assignment and declare will probably change a bit once built-in collections are added, so be prepared to chnage these tests as well.
        '''

        num1 = Literal('real-literal', '10.0')
        num2 = Literal('real-literal', '77.677')
        var1 = VariableLookup('fyodor')
        var2 = VariableLookup('mitya')
        bo1 = BinaryOp('*', var1, num2)
        bo2 = BinaryOp('+', var1, num2)

        self.assertEqual( Assign('foo', bo1), Assign('foo', bo1) )
        self.assertEqual( Declare('foo', num1), Declare('foo', num1) )
        self.assertNotEqual( Assign('foo', bo1), Declare('foo', num1))
        self.assertNotEqual( Assign('bar', bo1), Assign('foo', bo1) )
    
    def test_control_flow(self):
        '''Also tests the AssignOp Node type '''

        num1 = Literal('real-literal', '10.0')
        num2 = Literal('real-literal', '77.677')
        var1 = VariableLookup('fyodor')
        var2 = VariableLookup('mitya')
        bo1 = BinaryOp('==', var1, num2)
        bo2 = BinaryOp('!=', var1, num2)

        it1 = IfThen(bo1, AssignOp('beep', '+=', num1))
        it2 = IfThen(bo1, AssignOp('beep', '+=', num1))
        it3 = IfThen(bo1, AssignOp('beep', '+=', num2))

        ie1 = IfElse(bo2, AssignOp('bop', '-=', num2), AssignOp('beep', '+=', num1))
        ie2 = IfElse(bo2, AssignOp('bop', '-=', num2), AssignOp('beep', '+=', num1))
        ie3 = IfElse(bo1, AssignOp('bop', '-=', num2), AssignOp('beep', '+=', num1))

        w1 = While(bo1, AssignOp('beep', '+=', num1))
        w2 = While(bo1, AssignOp('beep', '*=', num1))
        w3 = While(bo2, AssignOp('beep', '+=', num1))
        w4 = While(bo1, AssignOp('beep', '+=', num1))

        self.assertEqual(it1, it2)
        self.assertEqual(ie1, ie2)
        self.assertEqual(w1, w4)
        self.assertNotEqual(it1, ie1)
        self.assertNotEqual(ie1, w2)
        self.assertNotEqual(w3, it2)
        self.assertNotEqual(it1, it3)
        self.assertNotEqual(ie1, ie3)
        self.assertNotEqual(w1, w2)
        self.assertNotEqual(w1, w3)

    def test_fn_declare(self):
        '''Also tests the return statement'''

        var1 = VariableLookup('fyodor')
        var2 = VariableLookup('mitya')

        fd1 = FunctionDeclare( 'getFyodor', [], Return(var1) )
        fd2 = FunctionDeclare( 'getFyodor', [], Return(var1) )
        fd3 = FunctionDeclare( 'getFyodor', ['a', 'b'], Return(var1) )
        fd4 = FunctionDeclare( 'getFyodor', ['a', 'c'], Return(var1) )
        fd5 = FunctionDeclare( 'getFyodor', ['a', 'b'], Return(var2) )
        fd6 = FunctionDeclare( 'getMitya', ['a', 'b'], Return(var2) )
        fd7 = FunctionDeclare( 'getMitya', ['a', 'b'], Return(var2) )

        self.assertEqual(fd1, fd2)
        self.assertEqual(fd6, fd7)
        self.assertNotEqual(fd3, fd4)
        self.assertNotEqual(fd3, fd5)
        self.assertNotEqual(fd5, fd6)

    def test_program_block(self):
        num1 = Literal('real-literal', '10.0')
        num2 = Literal('real-literal', '77.677')
        var1 = VariableLookup('fyodor')
        var2 = VariableLookup('mitya')
        bo1 = BinaryOp('*', var1, num2)
        bo2 = BinaryOp('+', var1, num2)
        a1 = Assign('foo', bo1)
        d1 = Declare('foo', num1)

        prgm1 = Program()
        prgm2 = Program()
        prgm3 = Program()
        prgm1.nodes = [a1, a1, d1]
        prgm2.nodes = [a1, a1, d1]
        prgm3.nodes = [a1, d1]

        blck1 = Block([a1, a1, d1])
        blck2 = Block([a1, a1, d1])
        blck3 = Block([a1])

        self.assertEqual(prgm1, prgm2)
        self.assertEqual(blck1, blck2)
        self.assertNotEqual(prgm1, prgm3)
        self.assertNotEqual(prgm1, blck1)
        self.assertNotEqual(blck2, blck3)





if __name__ == '__main__':
    unittest.main()