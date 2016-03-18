package com.ridgerun.gsttune.handlers;

import java.awt.Dimension;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;


/**
 * handler extends AbstractHandler, an IHandler base class.
 * @see org.eclipse.core.commands.IHandler
 * @see org.eclipse.core.commands.AbstractHandler
 */
public class ShowDotHandler extends AbstractHandler {
	
	private static final String COMMAND = "dot -Tpng /home/caguero/0.00.02.742694709-gst-launch.PLAYING_READY.dot -o /home/caguero/output.png";
	private static final String RESULTFILE = "/home/caguero/output.png";
	
	/**
	 * The constructor.
	 */
	public ShowDotHandler() {
	}

	/**
	 * the command has been executed, so extract extract the needed information
	 * from the application context.
	 */
	public Object execute(ExecutionEvent event) throws ExecutionException {
		
		Process p;
		try {
			p = Runtime.getRuntime().exec(COMMAND);
			p.waitFor();
		} catch (Exception e) {
			e.printStackTrace();
		}

		ImageIcon image = new ImageIcon(RESULTFILE);
        JLabel imageLabel = new JLabel(image);
		JFrame frame = new JFrame(""); 
        frame.add(imageLabel);
        frame.setSize(new Dimension(image.getIconWidth(), image.getIconHeight()));
        frame.setVisible(true);
        
        return null;
	}
}
