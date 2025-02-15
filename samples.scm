(define factorial 
  (lambda ((x i8)) i8
     (if (<= n 1)
         1
         (* n (factorial (- n 1))))))

(define (factorial ((x i8)) i8)
  (if (<= n 1)
      1
      (* n (factorial (- n 1)))))
    
(define (my-add ((x i8) (y i8)) i8)
  (+ x y))

(define (my-add x y)
  (+ x y))

(my-add 4 5)
(my-add 4 5.0)

(+ 100 50)
(- 100 50)
(* 50 10)
(/ 100 50)
(modulo 100 3)


(+ 100.0 50.0)
(- 100.0 50.0)
(* 50.0 10.0)
(/ 100.0 50.0)


(define x (vec-with-capacity i8 100))
(define y (vec-with-capacity f64 100))

(define x (hash-table (str i8)))


(define sqr (lambda (x) (* x x)))

(sqr 5)
(sqr 5.0)
(sqr (/ 1. 5.))
