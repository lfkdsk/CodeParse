../diksam array.dkm > array_.result 2>&1
../diksam cast.dkm > cast_.result 2>&1
../diksam class01.dkm > class01_.result 2>&1
../diksam class02.dkm > class02_.result 2>&1
../diksam class03.dkm > class03_.result 2>&1
../diksam classmain.dkm > classmain_.result 2>&1
../diksam downcast.dkm > downcast_.result 2>&1
../diksam instanceof.dkm > instanceof_.result 2>&1
../diksam method.dkm > method_.result 2>&1
../diksam shape.dkm > shape_.result 2>&1
../diksam shapemain.dkm > shapemain_.result 2>&1
../diksam super.dkm > super_.result 2>&1
../diksam test.dkm > test_.result 2>&1

diff array_.result array.result
diff cast_.result cast.result
diff class01_.result class01.result
diff class02_.result class02.result
diff class03_.result class03.result
diff classmain_.result classmain.result
diff downcast_.result downcast.result
diff instanceof_.result instanceof.result
diff method_.result method.result
diff shape_.result shape.result
diff shapemain_.result shapemain.result
diff super_.result super.result
diff test_.result test.result
