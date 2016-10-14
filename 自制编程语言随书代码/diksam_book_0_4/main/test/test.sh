../diksam test.dkm > test_.result 2>&1
../diksam array.dkm > array_.result 2>&1
../diksam class01.dkm > class01_.result 2>&1
../diksam class02.dkm > class02_.result 2>&1
../diksam class03.dkm > class03_.result 2>&1
../diksam method.dkm > method_.result 2>&1
../diksam cast.dkm > cast_.result 2>&1
../diksam classmain.dkm > classmain_.result 2>&1
../diksam downcast.dkm > downcast_.result 2>&1
../diksam instanceof.dkm > instanceof_.result 2>&1
../diksam super.dkm > super_.result 2>&1
../diksam exception.dkm > exception_.result 2>&1
../diksam shapemain.dkm > shapemain_.result 2>&1
../diksam throws.dkm > throws_.result 2>&1
../diksam nullpointer.dkm > nullpointer_.result 2>&1
../diksam array_ex.dkm > array_ex_.result 2>&1
../diksam else_ex.dkm > else_ex_.result 2>&1
../diksam native_pointer.dkm
../diksam switch.dkm > switch_.result 2>&1
../diksam final.dkm > final_.result 2>&1
../diksam do_while.dkm > do_while_.result 2>&1
../diksam enum.dkm > enum_.result 2>&1
../diksam delegate.dkm > delegate_.result 2>&1
../diksam rename.dkm > rename_.result 2>&1

echo "test"
diff test.result test_.result
echo "array"
diff array.result array_.result
echo "class01"
diff class01.result class01_.result
echo "class02"
diff class02.result class02_.result
echo "class03"
diff class03.result class03_.result
echo "method"
diff method.result method_.result
echo "cast"
diff cast.result cast_.result
echo "classmain"
diff classmain.result classmain_.result
echo "downcast"
diff downcast.result downcast_.result
echo "instanceof"
diff instanceof.result instanceof_.result
echo "super"
diff super.result super_.result
echo "exception"
diff exception.result exception_.result
echo "shapemain"
diff shapemain.result shapemain_.result
echo "throws"
diff throws.result throws_.result
echo "nullpointer"
diff nullpointer.result nullpointer_.result
echo "array_ex"
diff array_ex.result array_ex_.result
echo "else_ex"
diff else_ex.result else_ex_.result
echo "test"
diff test.dkm test.copy
echo "switch"
diff switch.result switch_.result
echo "final"
diff final.result final_.result
echo "do_while"
diff do_while.result do_while_.result
echo "enum"
diff enum.result enum_.result
echo "delegate"
diff delegate.result delegate_.result
echo "rename"
diff rename.result rename_.result
