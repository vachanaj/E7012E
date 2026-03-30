import Control.Monad.Cont (label)
main = do
  putStrLn "Hello, World!"
  putStrLn ("Please look at my favorite odd numbers: " ++ show (filter odd [10..20]))

doubleMe :: Num a => a -> a
doubleMe x = x + x

threeDifferent :: Eq a => a -> a -> a -> Bool
threeDifferent x y z = x /= y && y /= z && x /= z

--smallestKset l = [x| x<- nonEmptySubsets l, sum x < 5]
smallestKset l = take3 (sort' [sum x | x <- nonEmptySubsets l])

subsets :: [a] -> [[a]]
subsets [] = [[]]
subsets (x:xs) =
    rest ++ [x:ys | ys <- rest]
  where
    rest = subsets xs

take3 (a:b:c:_) = [a,b,c]
take3 xs        = xs

sort' [] = []
sort' (x:xs) = insert x (sort' xs)

insert x [] = [x]
insert x (y:ys)
  | x <= y    = x : y : ys
  | otherwise = y : insert x ys

  
nonEmptySubsets :: [a] -> [[a]]
nonEmptySubsets xs =
    [ys | ys <- subsets xs, not (null ys)]

