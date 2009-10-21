import Prelude hiding (putStr, getLine, print)
import System.IO.UTF8 
import Control.Monad
import Control.Concurrent
import System.IO hiding (putStr, getLine, print)
import Graphics.UI.Gtk
import Graphics.UI.Gtk.Gdk.Events
import Graphics.UI.Gtk.Abstract.Paned

-- Make lines from hatexmpp look beautiful
pretty :: String -> String
pretty t = t ++ "\n"

bufferGet :: TextBuffer -> IO String
bufferGet tb = do
	si <- textBufferGetStartIter tb
	ei <- textBufferGetEndIter tb
	textBufferGetText tb si ei True


bufferAdd :: TextBuffer -> String -> IO ()
bufferAdd tb s = do
	t <- bufferGet tb
	textBufferSetText tb (t ++ s)  

inputKeyPressed inputb e = if (eventKeyName e == "Return") && (notElem Shift $ eventModifier e)
	then do	
		t <- bufferGet inputb
		putStr t
		hFlush stdout
		textBufferSetText inputb ""
		return True
	else return False

main = do
	--args <- initGUI
	unsafeInitGUIForThreadedRTS
	--let (chatfile:_) = args
	
	hSetBuffering stdout $ BlockBuffering Nothing
	hSetBuffering stdin $ NoBuffering --LineBuffering

	w <- windowNew
	onDestroy w mainQuit
	windowSetTitle w "glovexmpp"

	logb <- textBufferNew Nothing
	logv <- textViewNewWithBuffer logb
	textViewSetEditable logv False
	textViewSetCursorVisible logv False

        inputb <- textBufferNew Nothing
        inputv <- textViewNewWithBuffer inputb
	textViewSetAcceptsTab inputv True
	onKeyPress inputv $ inputKeyPressed inputb
	--logp <- panelNew
	--inputp <- panelNew
	panels <- vPanedNew
	panedAdd1 panels logv
	panedAdd2 panels inputv
	panedSetPosition panels 50

	set w [ containerChild := panels ]
	widgetShowAll w
	--windowPresent w
	
	forkOS $ forever $ do 
		l <- getLine
		postGUISync $ bufferAdd logb $ pretty l

	mainGUI
