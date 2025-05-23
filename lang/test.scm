(define (my-add x y)
  (+ x y))

(fn my-add2 (x y z) int
  (+ x y))


(enum fmode
  readonly
  readwrite)

(fn fopen ((fname str) (mode fmode)) file)

(fopen :name "/home" :mode readonly)


(let
  ((f (fopen "/home/dman" readonly)))
  (map ()))


(fn fopen ((fname str "Path of file being opened")
            (mode fmode "Mode to open file in"))
  file
  (os:fopen fname str))


(fn fopen file ((fname str "Path of file being opened")
            (mode fmode "Mode to open file in"))
  (os:fopen fname str))


(fn
  :name fopen
  :type file
  :args ((:name path :type str :doc "Path to open")
          (:name mode :type fmode :doc "Mode to open file in"))
  :doc "open file for io."
  :body (progn
          (os::open name fmode)))

(def grammar
  ~{:ws (set " \t\r\f\n\0\v")
  :readermac (set "';~,|")
  :symchars (+ (range "09" "AZ" "az" "\x80\xFF") (set "!$%&*+-./:<?=>@^_"))
  :token (some :symchars)
  :hex (range "09" "af" "AF")
  :escape (* "\\" (+ (set `"'0?\abefnrtvz`)
                       (* "x" :hex :hex)
                       (* "u" [4 :hex])
                       (* "U" [6 :hex])
                       (error (constant "bad escape"))))
    :comment (* "#" (any (if-not (+ "\n" -1) 1)))
    :symbol :token
    :keyword (* ":" (any :symchars))
    :constant (* (+ "true" "false" "nil") (not :symchars))
    :bytes (* "\"" (any (+ :escape (if-not "\"" 1))) "\"")
                    :string :bytes
                    :buffer (* "@" :bytes)
                    :long-bytes {:delim (some "`")
                    :open (capture :delim :n)
                    :close (cmt (* (not (> -1 "`")) (-> :n) '(backmatch :n)) ,=)
                    :main (drop (* :open (any (if-not :close 1)) :close))}
                    :long-string :long-bytes
                    :long-buffer (* "@" :long-bytes)
                    :number (cmt (<- :token) ,scan-number)
                    :raw-value (+ :comment :constant :number :keyword
                                 :string :buffer :long-string :long-buffer
                                 :parray :barray :ptuple :btuple :struct :dict :symbol)
                    :value (* (any (+ :ws :readermac)) :raw-value (any :ws))
                    :root (any :value)
                    :root2 (any (* :value :value))
                    :ptuple (* "(" :root (+ ")" (error "")))
                    :btuple (* "[" :root (+ "]" (error "")))
                    :struct (* "{" :root2 (+ "}" (error "")))
                    :parray (* "@" :ptuple)
                    :barray (* "@" :btuple)
                    :dict (* "@" :struct)
                    :main :root})