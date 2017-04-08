
; atoms
(def {nil} {})
(def {true} 1)
(def {false} 0)

; function definition
(def {fun} (\ {args body} {def (head args) (\ (tail args) body)}))

(fun {len l} {if (== l {}) {0} {+ 1 (len (tail l))}})

(fun {reverse l} {if (== l {}) {{}} {join (reverse (tail l)) (head l)}})

(fun {unpack f l} {
  eval (join (list f) l)
})

(fun {pack f & xs} {f xs})

(def {curry} unpack)
(def {uncurry} pack)

(fun {do & l} {
  if (== l nil)
    {nil}
    {last l}
})

(fun {let b} {
  ((\ {_} b) ())
})

(fun {first l} {eval (head l)})

(fun {nth n l} {
  if (== n 0)
    { first l}
    { nth (- n 1) (tail l)}
})
(fun {last l} {
   nth (- (len l) 1) l
})

(fun {take n l} {
  if (== n 0)
    {nil}
    {join (head l) (take (- n 1) (tail l))}
})

(fun {drop n l} {
  if (== n 0)
    {l}
    {drop (- n 1) (tail l)}
})

(fun {split n l} {list (take n l) (drop n l)})

(fun {elem x l} {
  if (== l nil)
    {false}
    {if (== x (first l)) {true} {elem x (tail l)}}
})

(fun {map f l} {
  if (== l nil)
    {nil}
    {join (list (f (first l))) (map f (tail l))}
})

(fun {filter f l} {
  if (== l nil)
    {nil}
    {join (if (f (first l)) {head l} {nil}) (filter f (tail l))}
})

(fun {foldl f z l} {
  if (== l nil)
    {z}
    {foldl f (f z (first l)) (tail l)}
})

(fun {sum l} {foldl + 0 l})
(fun {product l} {foldl * 1 l})

