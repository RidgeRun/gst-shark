package com.ridgerun.gsttune.handlers;

import java.awt.Dimension;
import java.io.File;

import javax.swing.ImageIcon;
import javax.swing.JFileChooser;
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

	private static String COMMAND;
	private static String RESULTFILE = "/tmp/output.png";

	/**
	 * The constructor.
	 */
	public ShowDotHandler() {
	}

	/**
	 * the command has been executed, so extract extract the needed information
	 * from the application context.
	 */
	ImageIcon image;

	public Object execute(ExecutionEvent event) throws ExecutionException {

		JFrame frame = new JFrame("");
		JFileChooser fc = new JFileChooser();
		fc.setPreferredSize(new Dimension(800,600));
		int returnVal = fc.showOpenDialog(frame); //Where frame is the parent component

		File file = null;
		if (returnVal == JFileChooser.APPROVE_OPTION) {
		    file = fc.getSelectedFile();
		    //Now you have your file to do whatever you want to do
		} else {
		    //User did not choose a valid file
		}

		COMMAND = "dot -Tpng " + file.getAbsolutePath() + " -o " + RESULTFILE;

		Process p;
		try {
			p = Runtime.getRuntime().exec(COMMAND);
			p.waitFor();
		} catch (Exception e) {
			e.printStackTrace();
		}

		image = new ImageIcon(RESULTFILE);

        JLabel imageLabel = new JLabel(image);
        frame.add(imageLabel);
        frame.setSize(new Dimension(image.getIconWidth(), image.getIconHeight()));
        frame.setVisible(true);

        try {
            Thread.sleep(500);
        } catch(InterruptedException ex) {
            Thread.currentThread().interrupt();
        }

        image.getImage().flush();
        return null;
	}
}